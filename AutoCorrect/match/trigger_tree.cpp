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

        // The letters are not strictly the same.
        if (!isCaseSensitive && !other.isCaseSensitive &&  // If either is case sensitive, we can't perform case insensitive comparison.
            std::towlower(letter) == std::towlower(other.letter))
        {
            return true;
        }

        return false;
    }

    // NOTE: We don't need partial ordering, since we're already saying that case insensitive and same-if-lowered `Letter`s are equal.
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


// The agents for tracking the current possible matches.
struct Agent
{
    std::wstring stroke;  // TODO: Storing a stroke for each agent is extremely inefficient.
    const Node* node;
};


std::filesystem::path match_file;
Node trigger_tree_root;
std::atomic<bool> is_trigger_tree_available = false;
std::mutex trigger_tree_mutex;
std::condition_variable trigger_tree_cv;


std::jthread trigger_tree_thread;

void setup_trigger_tree(std::filesystem::path matchFile)
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

                // Discard any input received during the loading. A workaround for listener.clear() (there's no such function).
                InputMessage discard;
                while (listener.try_pop(discard))
                {}

                continue;
            }

            const auto [inputLetter, isBeingComposed] = listener.pop();

            // returns true if a trigger was found
            const auto lambdaAdvanceAgent = [inputLetter, isBeingComposed, &nextIterationAgents](const std::wstring& stroke, const std::map<Letter, Node>& childMap)
            {
                if (!is_trigger_tree_available.load())
                {
                    // Pretend a trigger was found so that the invalid agents are cleared & short-circuit the checks.
                    return true;
                }

                for (const auto& [letter, child] : childMap)
                {
                    if (letter != inputLetter)
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
                        return true;
                    }

                    // If the letter is being composed, only check for the triggers, don't advance the agents.
                    // ex - Typing '갃' should match '가' in the middle of the composition.
                    // But we should not advance the agents since doing so would fail to match any Korean letters which are composed more than 1 letter.
                    if (!isBeingComposed)
                    {
                        nextIterationAgents.emplace_back(stroke, &child);
                        // NOTE: multiple matches can happen, not breaking here
                    }
                }

                return false;
            };
            
            // Check for triggers in the root node first.
            const bool isTriggerFound = lambdaAdvanceAgent(L"", trigger_tree_root.children) ||
                std::ranges::any_of(agents, [lambdaAdvanceAgent, inputLetter](const auto& agent) { return lambdaAdvanceAgent(agent.stroke + inputLetter, agent.node->children); });

            if (isTriggerFound)
            {
                nextIterationAgents.clear();
            }

            if (!isBeingComposed || isTriggerFound)
            {
                agents.clear();
                std::swap(agents, nextIterationAgents);
            }
        }
    } };
}


std::jthread trigger_tree_constructor_thread;

void reconstruct_trigger_tree()
{
    struct EndingMetaData
    {
        std::wstring replace;
        unsigned int backspaceCount = 0;
    };

    struct TempNode
    {
        /// These are used for recording duplicates
        Match match;
        std::wstring trigger;

        /// This is used in the second iteration
        int parentIndex = -1;

        std::map<Letter, TempNode> children;  // empty == ending
        EndingMetaData endingMetaData;  // only valid if children is empty
    };

    struct Ending
    {
        int replaceStringIndex = -1;
        int replaceStringLength = 0;
        unsigned int backspaceCount = 0;
    };

    struct Link
    {
        int parentIndex = -1;
        int childStartIndex = -1;
        int childLength = 0;
        int endingIndex = -1;
    };

    trigger_tree_constructor_thread.request_stop();
    if (trigger_tree_constructor_thread.joinable())
    {
        trigger_tree_constructor_thread.join();
    }
    is_trigger_tree_available.store(false);

    trigger_tree_root = Node{};

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
        TempNode root;
        std::vector<std::pair<Match, std::wstring>> triggersOverwritten;
        for (const Match& match : matches)
        {
            const auto& [triggers, replace, isCaseSensitive] = match;
            for (const auto& trigger : triggers)
            {
                TempNode* node = &root;

                bool isTriggerOverwritten = false;
                // Make a node for each letter except the last one, that will be an 'ending node'.
                for (auto triggerIt = trigger.begin(); triggerIt != trigger.end() - 1; ++triggerIt)
                {
                    const wchar_t ch = *triggerIt;
                    auto [it, isNew] = node->children.try_emplace(Letter{ ch, isCaseSensitive && is_cased_alpha(ch) }, TempNode{ match, trigger });
                    // Same case with the last letter overwriting, but in a reverse order.
                    // The letters from here are not reachable anyway, so discard them altogether.
                    if (!isNew && it->second.children.empty())  
                    {
                        triggersOverwritten.emplace_back(match, trigger);
                        isTriggerOverwritten = true;
                        break;
                    }
                    node = &it->second;
                    STOP
                }
                if (isTriggerOverwritten)
                {
                    continue;
                }

                // The 'ending node'
                const wchar_t lastLetter = trigger.back();
                auto backspaceCount = static_cast<unsigned int>(trigger.size());
                // If the last letter is Korean, it's probably composed with more than 2 letters.
                // The backspace count should be adjusted accordingly.
                if ((L'가' <= lastLetter && lastLetter <= L'힣') ||
                    (L'ㄳ' <= lastLetter && lastLetter <= L'ㅢ'))
                {
                    backspaceCount += static_cast<int>(normalize_hangul(std::wstring{ lastLetter }).size()) - 1;
                }
                STOP

                // Since finding a match resets all the agents, we cannot advance further anyway. Therefore overwriting is fine.
                Letter letter{ lastLetter, isCaseSensitive&& is_cased_alpha(lastLetter) };
                if (node->children.contains(letter))
                {
                    TempNode& duplicate = node->children.at(letter);
                    if (duplicate.children.empty())
                    {
                        // If there is already an ending node, keep the existing one.
                        triggersOverwritten.emplace_back(match, trigger);
                        continue;
                    }
                    else
                    {
                        // TODO: Its children will be non-reachable, mark them as overwritten.
                        
                    }
                }
                node->children[letter] = { .endingMetaData = { replace, backspaceCount } };
                STOP
            }
            STOP
        }
        STOP
            
        std::vector<Link> tree;
        std::vector<Ending> endings;
        std::wstring replaceStrings;

        // TODO: Add comments
        std::queue<TempNode> nodes;
        nodes.push(root);
        while (!nodes.empty())
        {
            const int index = static_cast<int>(tree.size());

            TempNode& node = nodes.front();
            nodes.pop();
            tree.emplace_back(Link{ node.parentIndex });
            STOP

            if (node.children.empty())
            {
                tree.back().endingIndex = static_cast<int>(endings.size());

                const int replaceStringIndex = static_cast<int>(replaceStrings.size());
                replaceStrings.append(node.endingMetaData.replace);
                endings.emplace_back(replaceStringIndex, static_cast<int>(node.endingMetaData.replace.size()), node.endingMetaData.backspaceCount);
                STOP
            }

            Link& parent = tree.at(node.parentIndex);
            if (parent.childStartIndex < 0)
            {
                parent.childStartIndex = index;
            }
            parent.childLength++;
            STOP

            for (TempNode& child : node.children | std::views::values)
            {
                child.parentIndex = index;
                nodes.push(child);
                STOP
            }
            STOP
        }

        is_trigger_tree_available.store(true);
        trigger_tree_cv.notify_one();

#undef STOP
    } };
}

void teardown_trigger_tree()
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
