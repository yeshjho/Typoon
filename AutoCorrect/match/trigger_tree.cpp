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
            comp != std::strong_ordering::equal)
        {
            return comp;
        }

        return isCaseSensitive <=> other.isCaseSensitive;
    }
};

// A node of the tree. It's essentially a link, since a node doesn't hold any information.
struct Node
{
    int parentIndex = -1;
    int childStartIndex = -1;
    int childLength = 0;
    Letter letter;
    int endingIndex = -1;
};

// The last letter of a trigger, contains the information for the replacement string.
struct Ending
{
    int replaceStringIndex = -1;
    unsigned int replaceStringLength = 0;
    unsigned int backspaceCount = 0;
};

// The agents for tracking the current possible triggers.
struct Agent
{
    const Node* node = nullptr;
    int strokeStartIndex = -1;
};


std::filesystem::path match_file;

std::vector<Node> tree;
unsigned int tree_height;
std::vector<Ending> endings;
std::wstring replace_strings;

std::atomic<bool> is_trigger_tree_outdated = true;
std::atomic<bool> is_constructing_trigger_tree = false;


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
        std::vector<Agent> agents;
        std::vector<Agent> nextIterationAgents;
        std::wstring stroke;
        
        while (true)
        {
            if (stopToken.stop_requested()) [[unlikely]]
            {
                break;
            }

            const auto [inputLetter, isBeingComposed] = listener.pop();

            if (is_trigger_tree_outdated.load())
            {
                while (is_constructing_trigger_tree.load())
                {
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(10ms);
                }
                is_trigger_tree_outdated.store(false);

                // Discard any input received during the loading. A workaround for listener.clear() (there's no such function).
                InputMessage discard;
                while (listener.try_pop(discard))
                {}

                agents.clear();
                nextIterationAgents.clear();
                stroke.clear();
                agents.reserve(tree_height);
                nextIterationAgents.reserve(tree_height);
                stroke.resize(tree_height, 0);
            }

            std::ranges::rotate(stroke, stroke.begin() + 1);
            stroke.back() = inputLetter;
            for (Agent& agent : agents)
            {
                // NOTE: We don't need to handle the case the index goes negative
                // since those agents can't be made.
                agent.strokeStartIndex--;
            }

            // returns true if an ending was found
            const auto lambdaAdvanceAgent = [inputLetter, isBeingComposed, &nextIterationAgents](const Agent& agent)
            {
                if (is_trigger_tree_outdated.load())
                {
                    // Pretend a trigger was found so that the invalid agents are cleared & short-circuit the remaining checks.
                    return true;
                }

                const Node& node = *agent.node;
                // TODO: Maybe use binary search(std::equal_range)? Should modify the spaceship operator too, then.

                for (int i = node.childStartIndex; i < node.childStartIndex + node.childLength; i++)
                {
                    const Node& child = tree.at(i);
                    if (child.letter != inputLetter)
                    {
                        continue;
                    }

                    if (child.endingIndex >= 0)
                    {
                        const auto& [replaceStringIndex, replaceStringLength, backspaceCount] = endings.at(child.endingIndex);
                        const std::wstring_view replace{ replace_strings.data() + replaceStringIndex, replaceStringLength };

                        std::vector<FakeInput> inputs;
                        inputs.reserve(backspaceCount + replace.size());
                        std::fill_n(std::back_inserter(inputs), backspaceCount, FakeInput{ .type = FakeInput::EType::BACKSPACE });
                        std::ranges::transform(replace, std::back_inserter(inputs),
                            [](const wchar_t& ch) { return FakeInput{ FakeInput::EType::LETTER, ch }; });
                        send_fake_inputs(inputs);  // TODO: run it on a separate thread?

                        return true;
                    }
                    else
                    {
                        // If the letter is being composed, only check for the triggers, don't advance the agents.
                        // ex - Typing '갃' should match '가' in the middle of the composition.
                        // But we should not advance the agents since doing so would fail to match any Korean letters which are composed more than 1 letter.
                        if (!isBeingComposed)
                        {
                            nextIterationAgents.emplace_back(&child, agent.strokeStartIndex - 1);
                        }
                        // NOTE: multiple matches can happen(ex - case-sensitive one and non- one), hence not breaking
                    }
                }

                return false;
            };
            
            // Check for triggers in the root node first.
            Agent root{ &tree.front(), static_cast<int>(tree_height - 1) };
            const bool isTriggerFound = lambdaAdvanceAgent(root) ||
                std::ranges::any_of(agents, [lambdaAdvanceAgent, inputLetter](const Agent& agent) { return lambdaAdvanceAgent(agent); });

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
        const Match* match = nullptr;
        const std::wstring* trigger = nullptr;

        /// This is used in the second iteration
        int parentIndex = -1;
        const Letter* letter = nullptr;
        unsigned int height = 0;

        std::map<Letter, TempNode> children;  // empty == ending
        EndingMetaData endingMetaData;  // only valid if children is empty
    };

    trigger_tree_constructor_thread.request_stop();
    if (trigger_tree_constructor_thread.joinable())
    {
        trigger_tree_constructor_thread.join();
    }
    is_trigger_tree_outdated.store(true);
    is_constructing_trigger_tree.store(true);

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

        /// First iteration. Construct the tree, preprocessing the data to be easy to use.
        TempNode root;
        std::vector<std::pair<const Match*, const std::wstring*>> triggersOverwritten;
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
                    auto [it, isNew] = node->children.try_emplace(Letter{ ch, isCaseSensitive && is_cased_alpha(ch) }, TempNode{ &match, &trigger });
                    // Same case with the last letter overwriting, but in a reverse order.
                    // The letters from here are not reachable anyway, so discard them altogether.
                    if (!isNew && it->second.children.empty())
                    {
                        triggersOverwritten.emplace_back(&match, &trigger);
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
                        triggersOverwritten.emplace_back(&match, &trigger);
                        continue;
                    }
                    else
                    {
                        // Its children will be non-reachable, mark them as overwritten.
                        std::queue<TempNode*> nodes;
                        nodes.push(&duplicate);
                        while (!nodes.empty())
                        {
                            TempNode* childNode = nodes.front();
                            nodes.pop();
                            if (childNode->children.empty())
                            {
                                triggersOverwritten.emplace_back(childNode->match, childNode->trigger);
                            }
                            else
                            {
                                for (auto& child : childNode->children | std::views::values)
                                {
                                    nodes.push(&child);
                                    STOP
                                }
                            }
                            STOP
                        }
                    }
                }
                node->children[letter] = { .endingMetaData = { replace, backspaceCount } };
                STOP
            }
            STOP
        }
        STOP

        // TODO: Warn with triggersOverwritten

        tree.clear();
        endings.clear();
        replace_strings.clear();
        
        /// Second iteration. Actually build the tree which will be used at runtime.
        std::queue<TempNode*> nodes;
        nodes.push(&root);
        unsigned int height = 0;
        // Traverse the tree in level-order, so that all the links of a node to be contiguous.
        while (!nodes.empty())
        {
            const int index = static_cast<int>(tree.size());

            TempNode* node = nodes.front();
            nodes.pop();
            tree.emplace_back(Node{ .parentIndex = node->parentIndex });
            if (node->letter)
            {
                tree.back().letter = *node->letter;
            }
            height = std::max(height, node->height);
            STOP

            if (node->children.empty())
            {
                tree.back().endingIndex = static_cast<int>(endings.size());

                // TODO: Improve duplicate detection (ex - abc being added after bc is added)
                int replaceStringIndex = -1;
                if (const size_t result = replace_strings.find(node->endingMetaData.replace);
                    result == std::wstring::npos)
                {
                    replaceStringIndex = static_cast<int>(replace_strings.size());
                    replace_strings.append(node->endingMetaData.replace);
                }
                else
                {
                    replaceStringIndex = static_cast<int>(result);
                }

                endings.emplace_back(replaceStringIndex, static_cast<int>(node->endingMetaData.replace.size()), node->endingMetaData.backspaceCount);
                STOP
            }

            if (node->parentIndex >= 0)
            {
                Node& parent = tree.at(node->parentIndex);
                if (parent.childStartIndex < 0)
                {
                    parent.childStartIndex = index;
                }
                parent.childLength++;
            }
            STOP

            for (auto& [letter, child] : node->children)
            {
                child.parentIndex = index;
                child.letter = &letter;
                child.height = node->height + 1;
                nodes.push(&child);
                STOP
            }
            STOP
        }

        tree_height = height;
        is_constructing_trigger_tree.store(false);

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
