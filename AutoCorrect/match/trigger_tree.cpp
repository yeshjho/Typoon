#include "trigger_tree.h"

#include <cwctype>
#include <map>
#include <thread>

#include "../input_multicast/input_multicast.h"
#include "../low_level/fake_input.h"
#include "../parse/parse.h"
#include "../utils/logger.h"
#include "../utils/string.h"


struct Letter
{
    wchar_t letter;
    bool isCaseSensitive;  // Won't be true if `letter` is not cased.

    bool operator==(wchar_t ch) const
    {
        if (isCaseSensitive || !is_cased_alpha(ch))
        {
            return letter == ch;
        }

        return std::towlower(letter) == std::towlower(ch);
    }

    bool operator==(const Letter& other) const
    {
        if (letter == other.letter)
        {
            return isCaseSensitive == other.isCaseSensitive;
        }

        if (!isCaseSensitive && !other.isCaseSensitive && std::towlower(letter) == std::towlower(other.letter))
        {
            return true;
        }

        return false;
    }

    // NOTE: We don't need partial ordering, since we're already saying that case insensitive and same-if-lowered letters are equal.
    auto operator<=>(const Letter& other) const
    {
        if (std::towlower(letter) == std::towlower(other.letter))
        {
            // Case sensitive ones come first.
            return other.isCaseSensitive <=> isCaseSensitive;
        }

        if (const auto comp = letter <=> other.letter;
            comp != 0)
        {
            return comp;
        }
        return isCaseSensitive <=> other.isCaseSensitive;
    }
};

struct Node
{
    std::map<Letter, Node> children;
    bool isTrigger = false;
    std::wstring replace;
    unsigned int backspaceCount;
};


struct Agent
{
    std::wstring stroke;
    const Node* node;
};


std::filesystem::path match_file;
Node trigger_tree_root;
std::atomic<bool> is_trigger_tree_available = false;
std::mutex trigger_tree_mutex;
std::condition_variable trigger_tree_cv;


std::jthread trigger_tree_thread;

void initiate_trigger_tree(std::filesystem::path matchFile)
{
    if (trigger_tree_thread.joinable()) [[unlikely]]
    {
        g_console_logger.Log("Trigger tree is already running.", ELogLevel::WARNING);
        return;
    }

    match_file = std::move(matchFile);
    reconstruct_trigger_tree();
    
    trigger_tree_thread = std::jthread{ [&listener = register_input_listener()](const std::stop_token& stopToken)
    {
        std::vector<Agent> agents;  // TODO: Analyze the tree and reserve the max amount. Maybe use a pool?
        std::vector<Agent> nextIterationAgents;
        
        while (true)
        {
            if (stopToken.stop_requested()) [[unlikely]]
            {
                break;
            }

            if (!is_trigger_tree_available.load())
            {
                std::unique_lock lock{ trigger_tree_mutex };
                trigger_tree_cv.wait(lock);

                InputType discard;
                while (listener.try_pop(discard))
                {}

                continue;
            }

            const InputType input = listener.pop();

            const auto lambdaAdvanceAgent = [input, &nextIterationAgents](const std::wstring& stroke, const std::map<Letter, Node>& childMap)  // returns true if a trigger was found
            {
                for (const auto& [letter, child] : childMap)
                {
                    if (letter != input)
                    {
                        continue;
                    }

                    if (child.isTrigger)
                    {
                        std::vector<FakeInput> inputs;
                        inputs.reserve(child.backspaceCount + child.replace.size());
                        std::fill_n(std::back_inserter(inputs), child.backspaceCount, FakeInput{ .type = FakeInput::EType::BACKSPACE });
                        std::ranges::transform(child.replace, std::back_inserter(inputs),
                            [](wchar_t ch) { return FakeInput{ FakeInput::EType::LETTER, ch }; });
                        send_fake_inputs(inputs);  // TODO: run it on a separate thread?

                        nextIterationAgents.clear();
                        return true;
                    }

                    nextIterationAgents.emplace_back(stroke, &child);
                    // NOTE: multiple matches can happen, not breaking here
                }

                return false;
            };

            if (!lambdaAdvanceAgent(L"", trigger_tree_root.children))
            {
                for (auto& [stroke, node] : agents)
                {
                    if (lambdaAdvanceAgent(stroke + input, node->children))
                    {
                        break;
                    }
                }
            }

            agents.clear();
            std::swap(agents, nextIterationAgents);
        }
    } };
}


std::jthread trigger_tree_constructor_thread;

void reconstruct_trigger_tree()
{
    trigger_tree_root = Node{};

    trigger_tree_constructor_thread.request_stop();
    if (trigger_tree_constructor_thread.joinable())
    {
        trigger_tree_constructor_thread.join();
    }
    is_trigger_tree_available.store(false);
    trigger_tree_constructor_thread = std::jthread{ [](const std::stop_token& stopToken)
    {
#define STOP if (stopToken.stop_requested()) { return; }

        STOP
        auto&& matchesParsed = parse_matches(match_file);
        STOP
        std::vector<Match> matches;
        matches.reserve(matchesParsed.size());
        std::ranges::transform(matchesParsed, std::back_inserter(matches), [](const MatchForParse& match) { return match; });
        STOP
        // First iteration. Focus on 'constructing' the tree, runtime efficiency will be handled later.
        for (const auto& [triggers, replace, isCaseSensitive] : matches)
        {
            for (const auto& trigger : triggers)
            {
                std::wstring normalizedTrigger = normalize_hangul(trigger);
                Node* node = &trigger_tree_root;
                for (auto triggerIt = normalizedTrigger.begin(); triggerIt != normalizedTrigger.end() - 1; ++triggerIt)
                {
                    const wchar_t ch = *triggerIt;
                    auto [it, isNew] = node->children.try_emplace(Letter{ ch, isCaseSensitive && is_cased_alpha(ch) }, Node{});
                    if (!isNew && it->second.isTrigger)  // Same case with the last letter overwriting, but in a reverse order.
                    {
                        break;
                    }
                    node = &it->second;
                    STOP
                }

                const wchar_t lastLetter = trigger.back();
                const wchar_t lastNormalizedLetter = normalizedTrigger.back();
                auto backspaceCount = static_cast<unsigned int>(trigger.size());
                if (L'가' <= lastLetter && lastLetter <= L'힣')
                {
                    backspaceCount += static_cast<int>(normalize_hangul(std::wstring{ lastLetter }).size()) - 1;
                }
                // Since finding a match resets all the agents, we cannot advance further anyway. Therefore overwriting is fine.
                node->children[{ lastNormalizedLetter, isCaseSensitive && is_cased_alpha(lastNormalizedLetter) }] =
                    { .isTrigger = true, .replace = replace, .backspaceCount = backspaceCount };
                STOP
            }
            STOP
        }
        is_trigger_tree_available.store(true);
        trigger_tree_cv.notify_one();

#undef STOP
    } };

}

void terminate_trigger_tree()
{
    trigger_tree_thread.request_stop();
    if (trigger_tree_thread.joinable())
    {
        trigger_tree_thread.join();
    }
    trigger_tree_constructor_thread.request_stop();
    if (trigger_tree_constructor_thread.joinable())
    {
        trigger_tree_constructor_thread.join();
    }
}
