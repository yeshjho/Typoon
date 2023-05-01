#include "trigger_tree.h"

#include <cwctype>
#include <map>
#include <thread>

#include "../imm/imm_simulator.h"
#include "../input_multicast/input_multicast.h"
#include "../low_level/fake_input.h"
#include "../low_level/input_watcher.h"
#include "../parse/parse.h"
#include "../utils/logger.h"
#include "../utils/string.h"


struct Letter
{
    // These constants uses the unassigned block of the Unicode. (0x0870 ~ 0x089F)
    /// Constants for special triggers
    static constexpr wchar_t NON_WORD_LETTER = 0x0870;

    /// Constants for special replacements
    static constexpr wchar_t LAST_INPUT_LETTER = 0x089F;


    wchar_t letter;
    bool isCaseSensitive;  // Won't be true if `letter` is not cased.
    bool doNeedFullComposite;  // Won't be true if `letter` is not Korean or not an ending.

    bool operator==(wchar_t ch) const
    {
        if (letter == NON_WORD_LETTER)
        {
            return !std::iswalnum(ch);
        }

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
    std::strong_ordering operator<=>(const Letter& other) const
    {
        if (std::towlower(letter) == std::towlower(other.letter))
        {
            // Case sensitive ones come first.
            return other.isCaseSensitive <=> isCaseSensitive;
        }

        if (const std::strong_ordering comp = letter <=> other.letter;
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
    unsigned int cursorMoveCount = 0;
    bool propagateCase = false;  // Won't be true if the first letter is not cased.
    Match::EUppercaseStyle uppercaseStyle = Match::EUppercaseStyle::FIRST_LETTER;  // Only used if `propagateCase` is true.
    bool keepComposite = false;  // Won't be true if the letter is not Korean or need full composite.
};

// The agents for tracking the current possible triggers.
struct Agent
{
    const Node* node = nullptr;
    int strokeStartIndex = -1;

    constexpr bool operator==(const Agent& other) const noexcept = default;
};

// The agents that is 'dead' but can be brought back to life with backspaces.
struct DeadAgent : Agent
{
    int backspacesNeeded = 0;
};


std::filesystem::path match_file;

std::vector<Node> tree;
unsigned int tree_height;
std::vector<Ending> endings;
std::wstring replace_strings;

std::atomic<bool> is_trigger_tree_outdated = true;
std::atomic<bool> is_constructing_trigger_tree = false;


void replace_string(const Ending& ending, const Agent& agent, std::wstring_view stroke, const InputMessage(&inputs)[MAX_INPUT_COUNT], int inputLength, int inputIndex, bool doNeedFullComposite)
{
    const auto& [replaceStringIndex, replaceStringLength, backspaceCount, cursorMoveCount, propagateCase, uppercaseStyle, keepComposite] = ending;
    const std::wstring_view originalReplaceString{ replace_strings.data() + replaceStringIndex, replaceStringLength };
    std::wstring replaceString{ originalReplaceString };

    unsigned int additionalCursorMoveCount = 0;

    imm_simulator.ClearComposition();

    for (wchar_t& c : replaceString)
    {
        if (c == Letter::LAST_INPUT_LETTER)
        {
            c = inputs[inputIndex].letter;
        }
    }
    
    const std::wstring_view triggerStroke = stroke.substr(agent.strokeStartIndex);
    if (propagateCase)
    {
        const auto triggerFirstCasedLetter = std::ranges::find_if(triggerStroke, [](wchar_t c) { return is_cased_alpha(c); });
        if (std::iswupper(*triggerFirstCasedLetter))
        {
            if (std::ranges::any_of(triggerFirstCasedLetter, triggerStroke.end(), [](wchar_t c) { return is_cased_alpha(c) && std::iswlower(c); }))
            {
                // If there is a lowercase letter in the stroke, only the first cased letter is capitalized.
                if (const auto it = std::ranges::find_if(originalReplaceString, [](wchar_t c) { return is_cased_alpha(c); });
                    it != originalReplaceString.end())
                {
                    switch (uppercaseStyle)
                    {
                    case Match::EUppercaseStyle::FIRST_LETTER:
                        replaceString.at(it - originalReplaceString.begin()) = std::towupper(*it);
                        break;

                    case Match::EUppercaseStyle::WORDS:
                    {
                        bool shouldBeUpper = true;
                        for (wchar_t& c : replaceString)
                        {
                            if (shouldBeUpper && is_cased_alpha(c))
                            {
                                c = std::towupper(c);
                                shouldBeUpper = false;
                            }
                            else if (!std::iswalnum(c))
                            {
                                shouldBeUpper = true;
                            }
                        }
                        break;
                    }

                    default:
                        std::unreachable();
                    }
                }
            }
            else
            {
                // Otherwise, capitalize all the letters.
                std::ranges::transform(replaceString, replaceString.begin(), std::towupper);
            }
        }
    }

    const std::wstring_view replace{ replaceString };
    std::vector<FakeInput> fakeInputs;
    if (doNeedFullComposite)
    {
        const wchar_t lastLetter = inputs[inputLength - 1].letter;
        const bool isLastLetterKorean = is_korean(lastLetter);
        const bool didCompositionEndByAddingLetters = inputIndex + 1 < inputLength;
        unsigned int additionalBackspaceCount = std::max(inputLength - inputIndex - 2, 0) + static_cast<unsigned int>(didCompositionEndByAddingLetters);
        std::wstring lastLetterString{ lastLetter };
        if (isLastLetterKorean && didCompositionEndByAddingLetters)
        {
            const std::wstring lastLetterNormalized = normalize_hangeul(lastLetterString);
            lastLetterString = hangeul_to_alphabet(lastLetterNormalized, false);
            // The length of the middle letters + the last letter's decomposition.
            additionalBackspaceCount += static_cast<unsigned int>(lastLetterNormalized.size()) - 1;
            for (const wchar_t ch : lastLetterNormalized)
            {
                imm_simulator.AddLetter(ch);
            }
        }
        const bool shouldToggleHangeul = isLastLetterKorean && !is_hangeul_on;

        // Note that we're not using the backspaceCount from the ending,
        // since the last letter of the replace string was decomposed to calculate the count (we don't want that here).
        const unsigned int totalBackspaceCount = tree_height - agent.strokeStartIndex + additionalBackspaceCount;
        fakeInputs.reserve(
            totalBackspaceCount + 
            replaceStringLength + 
            inputLength - inputIndex - 2 + 
            static_cast<int>(shouldToggleHangeul) + 
            (didCompositionEndByAddingLetters ? lastLetterString.size() : size_t{ 0 }));

        std::fill_n(std::back_inserter(fakeInputs), totalBackspaceCount, FakeInput{ FakeInput::EType::KEY, FakeInput::BACKSPACE_KEY });

        // The replace string first
        std::ranges::transform(replace, std::back_inserter(fakeInputs),
            [](const wchar_t& ch) { return FakeInput{ FakeInput::EType::LETTER, ch }; });
        // Then the middle letters
        // Not using std::transform to avoid begin(i + 1) > end(length - 1)
        for (int j = inputIndex + 1; j < inputLength - 1; j++)
        {
            fakeInputs.emplace_back(FakeInput::EType::LETTER, inputs[j].letter);
        }

        // Then the last letter's decomposition.
        // Why decompose and send as a key? If the last letter was in the middle of composing, we need to keep the 'composing' state.
        if (shouldToggleHangeul)
        {
            fakeInputs.emplace_back(FakeInput::EType::KEY, FakeInput::TOGGLE_HANGEUL_KEY);
            is_hangeul_on = true;
        }
        if (didCompositionEndByAddingLetters)
        {
            std::ranges::transform(lastLetterString, std::back_inserter(fakeInputs),
                [type = isLastLetterKorean ? FakeInput::EType::LETTER_AS_KEY : FakeInput::EType::LETTER](const wchar_t& ch)
                { return FakeInput{ type, ch }; });
        }

        if (didCompositionEndByAddingLetters && cursorMoveCount > 0)
        {
            additionalCursorMoveCount += inputLength - inputIndex - 1;
        }

        // NOTE: We don't skip the remaining letters since that can be a trigger, too
        // ex - matches: '가'(full composite) -> '다', '나' -> '라' / input: '가나' / replace should be: '다라'
    }
    else if (keepComposite)
    {
        const std::wstring lastLetterNormalized = normalize_hangeul(replace.substr(replaceStringLength - 1));
        const std::wstring lastLetter = hangeul_to_alphabet(lastLetterNormalized, false);

        fakeInputs.reserve(backspaceCount + replaceStringLength + static_cast<int>(!is_hangeul_on) + lastLetter.size() - 1);
        std::fill_n(std::back_inserter(fakeInputs), backspaceCount, FakeInput{ FakeInput::EType::KEY, FakeInput::BACKSPACE_KEY });
        std::ranges::transform(replace.substr(0, replaceStringLength - 1), std::back_inserter(fakeInputs),
            [](const wchar_t& ch) { return FakeInput{ FakeInput::EType::LETTER, ch }; });
        if (!is_hangeul_on)
        {
            fakeInputs.emplace_back(FakeInput::EType::KEY, FakeInput::TOGGLE_HANGEUL_KEY);
            is_hangeul_on = true;
        }
        std::ranges::transform(lastLetter, std::back_inserter(fakeInputs),
            [](const wchar_t& ch) { return FakeInput{ FakeInput::EType::LETTER_AS_KEY, ch }; });
        for (const wchar_t ch : lastLetterNormalized)
        {
            imm_simulator.AddLetter(ch);
        }
    }
    else
    {
        fakeInputs.reserve(backspaceCount + replaceStringLength);
        std::fill_n(std::back_inserter(fakeInputs), backspaceCount, FakeInput{ FakeInput::EType::KEY, FakeInput::BACKSPACE_KEY });
        std::ranges::transform(replace, std::back_inserter(fakeInputs),
            [](const wchar_t& ch) { return FakeInput{ FakeInput::EType::LETTER, ch }; });
    }

    std::fill_n(std::back_inserter(fakeInputs), cursorMoveCount + additionalCursorMoveCount, FakeInput{ FakeInput::EType::KEY, FakeInput::LEFT_ARROW_KEY });

    send_fake_inputs(fakeInputs, false);
}


void on_input(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents)
{
    constexpr int MAX_BACKSPACE_COUNT = 5;

    static std::vector<Agent> agents{};
    static std::vector<Agent> nextIterationAgents{};
    static std::deque<DeadAgent> deadAgents{};
    static std::wstring stroke{};
    static bool shouldResetAgents = false;
    static Agent root;

    if (is_constructing_trigger_tree.load())
    {
        shouldResetAgents = true;
        return;
    }

    if (shouldResetAgents || is_trigger_tree_outdated.load())
    {
        shouldResetAgents = false;
        is_trigger_tree_outdated.store(false);

        agents.clear();
        nextIterationAgents.clear();
        stroke.clear();
        agents.reserve(tree_height);
        nextIterationAgents.reserve(tree_height);
        stroke.resize(std::max(tree_height, 1U), 0);
        root = { &tree.front(), static_cast<int>(tree_height) };
    }

    if (clearAllAgents)
    {
        agents.clear();
        deadAgents.clear();
    }

    for (int i = 0; i < length; i++)
    {
        g_console_logger.Log(ELogLevel::DEBUG, "input:", inputs[i].letter, static_cast<int>(inputs[i].isBeingComposed));
    }

    if (length >= 0 && inputs[0].letter == L'\b')
    {
        std::ranges::shift_right(stroke, 1);

        // The input size is bigger than 1 only if letters are composed in the imm simulator.
        // But a backspace can't be used to finish composing(other than clearing one completely),
        // we don't need to check further.
        for (const auto [node, strokeStartIndex] : agents)
        {
            if (node->parentIndex >= 0)
            {
                nextIterationAgents.emplace_back(&tree.at(node->parentIndex), strokeStartIndex + 1);
            }
        }
        agents.clear();
        std::swap(agents, nextIterationAgents);

        std::erase_if(deadAgents,
            [](DeadAgent& deadAgent)
            {
                deadAgent.backspacesNeeded--;
                if (deadAgent.backspacesNeeded <= 0)
                {
                    agents.emplace_back(deadAgent);
                    return true;
                }
                return false;
            }
        );

        return;
    }

    // returns true if an ending was found
    const auto lambdaAdvanceAgent = [length, inputs](const Agent& agent, wchar_t inputLetter, bool isBeingComposed, int inputIndex)
    {
        if (is_trigger_tree_outdated.load())
        {
            // Pretend a trigger was found so that the invalid agents are cleared & short-circuit the remaining checks.
            return true;
        }

        const Node& node = *agent.node;
        // TODO: Maybe use binary search(std::equal_range)? Should modify the spaceship operator too, then.

        bool didFindMatchingChild = false;
        for (int childIndex = node.childStartIndex; childIndex < node.childStartIndex + node.childLength; childIndex++)
        {
            const Node& child = tree.at(childIndex);
            if (child.letter != inputLetter)
            {
                continue;
            }

            Agent nextAgent{ &child, agent.strokeStartIndex - 1 };
            if (child.endingIndex < 0)
            {
                // If the letter is being composed, only check for the triggers, don't advance the agents.
                // ex - Typing '갃' should match '가' in the middle of the composition.
                // But we should not advance the agents since doing so would fail to match any Korean letters which are composed more than 1 letter.
                if (!isBeingComposed)
                {
                    nextIterationAgents.emplace_back(nextAgent);
                    didFindMatchingChild = true;
                }
                // NOTE: multiple matches can happen(ex - case-sensitive one and non- one), hence not breaking
                continue;
            }

            const bool doNeedFullComposite = child.letter.doNeedFullComposite;
            if (doNeedFullComposite && isBeingComposed)
            {
                continue;
            }

            replace_string(endings.at(child.endingIndex), nextAgent, stroke, inputs, length, inputIndex, doNeedFullComposite);

            return true;
        }

        if (!didFindMatchingChild && agent != root)
        {
            deadAgents.emplace_back(agent);
        }
        return false;
    };

    for (int i = 0; i < length; i++)
    {
        const auto [inputLetter, isBeingComposed] = inputs[i];

        if (!isBeingComposed)
        {
            std::ranges::shift_left(stroke, 1);
            stroke.back() = inputLetter;
        }

        // Check for triggers in the root node first.
        bool isTriggerFound = lambdaAdvanceAgent(root, inputLetter, isBeingComposed, i);
        if (!isTriggerFound)
        {
            for (const Agent& agent : agents)
            {
                isTriggerFound = lambdaAdvanceAgent(agent, inputLetter, isBeingComposed, i);
                if (isTriggerFound)
                {
                    break;
                }
            }
        }

        if (isTriggerFound)
        {
            nextIterationAgents.clear();
            deadAgents.clear();
        }

        if (!isBeingComposed)
        {
            std::erase_if(deadAgents, [](DeadAgent& deadAgent) { return ++deadAgent.backspacesNeeded >= MAX_BACKSPACE_COUNT; });
        }

        if (!isBeingComposed || isTriggerFound)
        {
            agents.clear();
            std::swap(agents, nextIterationAgents);
        }
    }
}


void setup_trigger_tree(std::filesystem::path matchFile)
{
    static bool didSetup = false;
    if (didSetup)
    {
        g_console_logger.Log("Trigger tree is already running.", ELogLevel::WARNING);
        return;
    }
    didSetup = true;
    
    match_file = std::move(matchFile);
    reconstruct_trigger_tree();

    register_input_listener(on_input);
}


std::jthread trigger_tree_constructor_thread;

void reconstruct_trigger_tree()
{
    constexpr wchar_t CURSOR_PLACEHOLDER[] = L"|_|";

    struct EndingMetaData
    {
        std::wstring replace;
        Ending tempEnding;
    };

    struct TempNode
    {
        /// These are used for recording duplicates
        const Match* match = nullptr;
        const std::wstring* originalTrigger = nullptr;
        std::wstring trigger;

        /// This is used in the second iteration
        int parentIndex = -1;
        const Letter* letter = nullptr;
        unsigned int height = 0;

        std::map<Letter, TempNode> children{};  // empty == ending
        EndingMetaData endingMetaData{};  // only valid if children is empty
    };

    trigger_tree_constructor_thread.request_stop();
    if (trigger_tree_constructor_thread.joinable())
    {
        trigger_tree_constructor_thread.join();
    }
    is_trigger_tree_outdated.store(true);
    is_constructing_trigger_tree.store(true);

    g_console_logger.Log("Trigger tree construction started");

    trigger_tree_constructor_thread = std::jthread{ [](const std::stop_token& stopToken)
    {
#define STOP if (stopToken.stop_requested()) { return; }

        STOP
        std::vector<MatchForParse>&& matchesParsed = parse_matches(match_file);
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
            const auto& [triggers, originalReplace, isCaseSensitive, isWord, doPropagateCase, uppercaseStyle, doNeedFullComposite, doKeepComposite] = match;
            for (const auto& originalTrigger : triggers)
            {
                std::wstring triggerStr{ originalTrigger };
                std::wstring replaceStr{ originalReplace };

                if (isWord)
                {
                    triggerStr.push_back(Letter::NON_WORD_LETTER);
                    replaceStr.push_back(Letter::LAST_INPUT_LETTER);
                }

                unsigned int cursorMoveCount = 0;
                if (const size_t cursorIndex = originalReplace.find(CURSOR_PLACEHOLDER);
                    cursorIndex != std::wstring::npos)
                {
                    replaceStr.erase(cursorIndex, 3);
                    cursorMoveCount = static_cast<unsigned int>(replaceStr.size() - cursorIndex);
                }

                const std::wstring_view trigger = triggerStr;
                const std::wstring_view replace = replaceStr;

                TempNode* node = &root;

                bool isTriggerOverwritten = false;
                // Make a node for each letter except the last one, that will be an 'ending node'.
                for (auto triggerIt = trigger.begin(); triggerIt != trigger.end() - 1; ++triggerIt)
                {
                    const wchar_t ch = *triggerIt;
                    auto [it, isNew] = node->children.try_emplace(
                        Letter{ ch, isCaseSensitive && is_cased_alpha(ch), false }, 
                        TempNode{ &match, &originalTrigger, std::wstring{ trigger } });
                    // Same case with the last letter overwriting, but in a reverse order.
                    // The letters from here are not reachable anyway, so discard them altogether.
                    if (!isNew && it->second.children.empty())
                    {
                        triggersOverwritten.emplace_back(&match, &originalTrigger);
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
                const wchar_t triggerLastLetter = trigger.back();
                auto backspaceCount = static_cast<unsigned int>(trigger.size());
                const bool isTriggerLastLetterKorean = is_korean(triggerLastLetter);
                // If the last letter is Korean, it's probably composed with more than 2 letters.
                // The backspace count should be adjusted accordingly.
                if (isTriggerLastLetterKorean)
                {
                    backspaceCount += static_cast<int>(normalize_hangeul(std::wstring_view{ &triggerLastLetter, 1 }).size()) - 1;
                }
                STOP

                // Since finding a match resets all the agents, we cannot advance further anyway. Therefore overwriting is fine.
                const bool needFullComposite = doNeedFullComposite && isTriggerLastLetterKorean && !isWord;
                Letter letter{ triggerLastLetter, isCaseSensitive && is_cased_alpha(triggerLastLetter), needFullComposite };
                if (node->children.contains(letter))
                {
                    TempNode& duplicate = node->children.at(letter);
                    if (duplicate.children.empty())
                    {
                        // If there is already an ending node, keep the existing one.
                        triggersOverwritten.emplace_back(&match, &originalTrigger);
                        continue;
                    }

                    // Its children will be non-reachable, mark them as overwritten.
                    std::queue<TempNode*> nodes;
                    nodes.push(&duplicate);
                    while (!nodes.empty())
                    {
                        TempNode* childNode = nodes.front();
                        nodes.pop();
                        if (childNode->children.empty())
                        {
                            triggersOverwritten.emplace_back(childNode->match, childNode->originalTrigger);
                        }
                        else
                        {
                            for (TempNode& child : childNode->children | std::views::values)
                            {
                                nodes.push(&child);
                                STOP
                            }
                        }
                        STOP
                    }
                }

                Ending ending{
                    .backspaceCount = backspaceCount,
                    .cursorMoveCount = cursorMoveCount,
                    // TODO: Abstract the extra conditions of the options and warn the user if ignored
                    .propagateCase = doPropagateCase && !isCaseSensitive && std::ranges::any_of(trigger, [](wchar_t c) { return is_cased_alpha(c); }),
                    .uppercaseStyle = uppercaseStyle,
                    .keepComposite = doKeepComposite && is_korean(replace.back()) && !needFullComposite && cursorMoveCount == 0,
                };
                node->children[letter] = TempNode{ .endingMetaData = EndingMetaData{ .replace = std::wstring{ replace }, .tempEnding = ending } };
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

                Ending ending = node->endingMetaData.tempEnding;
                ending.replaceStringIndex = replaceStringIndex;
                ending.replaceStringLength = static_cast<int>(node->endingMetaData.replace.size());
                endings.emplace_back(ending);
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

        g_console_logger.Log("Trigger tree construction finished");

#undef STOP
    } };
}

void teardown_trigger_tree()
{
    trigger_tree_constructor_thread.request_stop();
    if (trigger_tree_constructor_thread.joinable())
    {
        trigger_tree_constructor_thread.join();
    }
}
