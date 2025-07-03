#include "trigger_tree.h"

#include <cwctype>
#include <map>

#include "../imm/imm_simulator.h"
#include "../low_level/clipboard.h"
#include "../low_level/command.h"
#include "../low_level/fake_input.h"
#include "../low_level/input_watcher.h"
#include "../low_level/tray_icon.h"
#include "../parse/parse_match.h"
#include "../utils/config.h"
#include "../utils/logger.h"
#include "../utils/string.h"


bool Letter::operator==(wchar_t ch) const
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


bool Letter::operator==(const Letter& other) const
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
std::strong_ordering Letter::operator<=>(const Letter& other) const
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


TriggerTree::TriggerTree(std::filesystem::path matchFile, std::vector<std::filesystem::path> includes, std::vector<std::filesystem::path> excludes)
    : mMatchFile(std::move(matchFile))
    , mIncludes(std::move(includes))
    , mExcludes(std::move(excludes))
{}


TriggerTree::~TriggerTree()
{
    HaltConstruction();
    for (const std::filesystem::path& file : mImportedFiles)
    {
        std::erase(trigger_trees_by_match_file.at(file), this);
    }
}


void TriggerTree::Reconstruct(std::string_view matchesString, std::function<void()> onFinish)
{
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

    HaltConstruction();
    mIsTriggerTreeOutdated.store(true);
    mIsConstructingTriggerTree.store(true);

    logger.Log(ELogLevel::INFO, mMatchFile, "Trigger tree construction started");

    mTriggerTreeConstructorThread = std::jthread{
        [this,
        matchesString = std::string{ matchesString.begin(), matchesString.end() },
        onFinish = std::move(onFinish),
        didCallOnFinish = false](const std::stop_token& stopToken) mutable
    {
        #define STOP if (stopToken.stop_requested()) { if (onFinish && !didCallOnFinish) { didCallOnFinish = true; onFinish(); } return; }

        STOP
        std::vector<MatchForParse> matchesParsed;
        if (matchesString.empty())
        {
            auto&& [matches, files] = parse_matches(mMatchFile, mIncludes, mExcludes);
            matchesParsed = std::move(matches);
            for (const std::filesystem::path& file : mImportedFiles)
            {
                std::erase(trigger_trees_by_match_file.at(file), this);
            }
            mImportedFiles = std::move(files);
            for (const std::filesystem::path& file : mImportedFiles)
            {
                trigger_trees_by_match_file[file].emplace_back(this);
            }
        }
        else
        {
            matchesParsed = parse_matches(std::string_view{ matchesString });
        }
        STOP
        std::vector<Match> matches;
        matches.reserve(matchesParsed.size());
        std::ranges::transform(matchesParsed, std::back_inserter(matches), [](const MatchForParse& match) { return match; });
        STOP
        std::ranges::filter_view matchesFiltered{ matches, [](const Match& match)
            {
                return (!match.triggers.empty() || !match.regexTrigger.empty()) && (!match.replace.empty() || !match.replaceImage.empty() || !match.replaceCommand.empty());
            }
        };
        // TODO: Warn about empty triggers or replaces

        /// First iteration. Construct the tree, preprocessing the data to be easy to use.
        TempNode root;
        std::vector<std::pair<const Match*, const std::wstring*>> triggersOverwritten;
        for (const Match& match : matchesFiltered)
        {
            const auto& [originalTriggers, regexTrigger, originalReplace, replaceImage, replaceCommand,
                isCaseSensitive, isWord, doPropagateCase, uppercaseStyle, 
                doNeedFullComposite, doKeepComposite, isKorEngInsensitive] = match;

            std::vector<std::wstring> triggers;
            if (isKorEngInsensitive)
            {
                triggers.reserve(originalTriggers.size() * 2);
                for (const std::wstring& originalTrigger : originalTriggers)
                {
                    // A trigger could be mixed with Korean and English letters.
                    triggers.emplace_back(combine_hangeul(alphabet_to_hangeul(originalTrigger)));
                    triggers.emplace_back(hangeul_to_alphabet(decompose_hangeul(originalTrigger), false));
                }
            }
            else
            {
                // A copy that could be avoided, but I think it's OK because normally there won't be many triggers,
                // and each of them won't be too long.
                triggers = originalTriggers;
            }

            std::wstring replaceStr{ originalReplace };
            if (isWord)
            {
                replaceStr.push_back(Letter::LAST_INPUT_LETTER);
            }

            unsigned int cursorMoveCount = 0;
            const std::wstring& cursorPlaceholder = get_config().cursorPlaceholder;
            if (const size_t cursorIndex = originalReplace.find(cursorPlaceholder);
                cursorIndex != std::wstring::npos)
            {
                replaceStr.erase(cursorIndex, cursorPlaceholder.size());
                cursorMoveCount = static_cast<unsigned int>(replaceStr.size() - cursorIndex);
            }

            const std::wstring_view replace = replaceStr;

            const Ending::EReplaceType replaceType =
                !replaceImage.empty() ? Ending::EReplaceType::IMAGE :
                !replaceCommand.empty() ? Ending::EReplaceType::COMMAND :
                Ending::EReplaceType::TEXT;
            const Ending endingBase{
                .type = replaceType,
                .cursorMoveCount = cursorMoveCount,
                // TODO: Abstract the extra conditions of the options and warn the user if ignored
                .propagateCase = doPropagateCase && !isCaseSensitive && replaceType == Ending::EReplaceType::TEXT,
                .uppercaseStyle = uppercaseStyle,
                .keepComposite = doKeepComposite && is_korean(replace.back()) && replaceType == Ending::EReplaceType::TEXT,
            };

            const EndingMetaData endingMetaDataBase{
                .replace =
                    replaceType == Ending::EReplaceType::IMAGE ? replaceImage.generic_wstring() :
                    replaceType == Ending::EReplaceType::COMMAND ? replaceCommand :
                    std::wstring{ replace },
            };

            for (const auto& originalTrigger : triggers)
            {
                std::wstring triggerStr{ originalTrigger };

                if (isWord)
                {
                    triggerStr.push_back(Letter::NON_WORD_LETTER);
                }

                const std::wstring_view trigger = triggerStr;

                TempNode* node = &root;

                bool isTriggerOverwritten = false;
                // Make a node for each letter except the last one, that will be an 'ending node'.
                for (auto triggerIt = trigger.begin(); triggerIt != trigger.end() - 1; ++triggerIt)
                {
                    const wchar_t ch = *triggerIt;
                    auto [it, isNew] = node->children.try_emplace(
                        Letter{
                            .letter = ch,
                            .isCaseSensitive = isCaseSensitive && is_cased_alpha(ch),
                            .doNeedFullComposite = false
                        },
                        TempNode{
                            .match = &match,
                            .originalTrigger = &originalTrigger,
                            .trigger = std::wstring{ trigger }
                        });
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
                    backspaceCount += static_cast<int>(decompose_hangeul(std::wstring_view{ &triggerLastLetter, 1 }).size()) - 1;
                }
                STOP

                // Since finding a match resets all the agents, we cannot advance further anyway. Therefore overwriting is fine.
                const bool needFullComposite = doNeedFullComposite && isTriggerLastLetterKorean && !isWord && !isKorEngInsensitive;
                Letter letter{
                    .letter = triggerLastLetter,
                    .isCaseSensitive = isCaseSensitive && is_cased_alpha(triggerLastLetter) && !isKorEngInsensitive,
                    .doNeedFullComposite = needFullComposite
                };
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

                // TODO: Abstract the extra conditions of the options and warn the user if ignored
                Ending ending = endingBase;
                ending.backspaceCount = backspaceCount;
                ending.propagateCase &= std::ranges::any_of(trigger, [](wchar_t c) { return is_cased_alpha(c); });
                ending.keepComposite &= !needFullComposite && cursorMoveCount == 0 && (trigger.size() > 1 || triggerLastLetter != replace.back());

                EndingMetaData endingMetaData = endingMetaDataBase;
                endingMetaData.tempEnding = ending;

                node->children[letter] = TempNode{ .endingMetaData = endingMetaData };
                STOP
            }
            STOP
        }
        STOP

        // TODO: Warn with triggersOverwritten

        mTree.clear();
        mEndings.clear();
        mReplaceStrings.clear();
        
        /// Second iteration. Actually build the tree which will be used at runtime.
        std::queue<TempNode*> nodes;
        nodes.push(&root);
        unsigned int height = 0;
        // Traverse the tree in level-order, so that all the links of a node to be contiguous.
        while (!nodes.empty())
        {
            const int index = static_cast<int>(mTree.size());

            TempNode* node = nodes.front();
            nodes.pop();
            mTree.emplace_back(Node{ .parentIndex = node->parentIndex });
            if (node->letter)
            {
                mTree.back().letter = *node->letter;
            }
            height = std::max(height, node->height);
            STOP

            if (node->children.empty())
            {
                mTree.back().endingIndex = static_cast<int>(mEndings.size());

                // Improving on the duplicate detection turned out to be a NP-hard problem, it's known as the 'shortest common superstring problem'.
                // Since the memory usage is not the first priority, it'll be fine with a simple solution like this.
                int replaceStringIndex = -1;
                if (const size_t result = mReplaceStrings.find(node->endingMetaData.replace);
                    result == std::wstring::npos)
                {
                    replaceStringIndex = static_cast<int>(mReplaceStrings.size());
                    mReplaceStrings.append(node->endingMetaData.replace);
                }
                else
                {
                    replaceStringIndex = static_cast<int>(result);
                }

                Ending ending = node->endingMetaData.tempEnding;
                ending.replaceStringIndex = replaceStringIndex;
                ending.replaceStringLength = static_cast<int>(node->endingMetaData.replace.size());
                mEndings.emplace_back(ending);
                STOP
            }

            if (node->parentIndex >= 0)
            {
                Node& parent = mTree.at(node->parentIndex);
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

        mTreeHeight = height;
        mIsConstructingTriggerTree.store(false);
        if (onFinish && !didCallOnFinish)
        {
            didCallOnFinish = true;
            onFinish();
        }

        logger.Log(ELogLevel::INFO, mMatchFile, "Trigger tree construction finished");

#undef STOP
    } };
}


void TriggerTree::ReconstructWith(std::filesystem::path matchFile)
{
    mMatchFile = std::move(matchFile);
    Reconstruct();
}


void TriggerTree::HaltConstruction()
{
    mTriggerTreeConstructorThread.request_stop();
    if (mTriggerTreeConstructorThread.joinable())
    {
        mTriggerTreeConstructorThread.join();
    }
}


void TriggerTree::WaitForConstruction() const
{
    while (mIsConstructingTriggerTree.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
    }
}


void TriggerTree::ResetAgents()
{
    mShouldResetAgents = true;
}


void TriggerTree::OnInput(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents)
{
    if (mIsConstructingTriggerTree.load())
    {
        mShouldResetAgents = true;
        return;
    }

    if (mShouldResetAgents || mIsTriggerTreeOutdated.load())
    {
        mShouldResetAgents = false;
        mIsTriggerTreeOutdated.store(false);

        mAgents.clear();
        mNextIterationAgents.clear();
        mStroke.clear();
        mAgents.reserve(mTreeHeight);
        mNextIterationAgents.reserve(mTreeHeight);
        mStroke.resize(std::max(mTreeHeight, 1U), 0);
        mRootAgent = { .node = &mTree.front(), .strokeStartIndex = static_cast<int>(mTreeHeight) };
    }

    if (clearAllAgents)
    {
        mAgents.clear();
        mDeadAgents.clear();
    }

    for (int i = 0; i < length; i++)
    {
        logger.Log(ELogLevel::DEBUG, "input:", inputs[i].letter, static_cast<int>(inputs[i].isBeingComposed));
    }

    if (length >= 0 && inputs[0].letter == L'\b')
    {
        std::ranges::shift_right(mStroke, 1);

        // The input size is bigger than 1 only if letters are composed in the imm simulator.
        // But a backspace can't be used to finish composing(other than clearing one completely),
        // we don't need to check further.
        for (const auto [node, strokeStartIndex] : mAgents)
        {
            if (node->parentIndex >= 0)
            {
                mNextIterationAgents.emplace_back(&mTree.at(node->parentIndex), strokeStartIndex + 1);
            }
        }
        mAgents.clear();
        std::swap(mAgents, mNextIterationAgents);

        std::erase_if(mDeadAgents,
            [this](DeadAgent& deadAgent)
            {
                deadAgent.backspacesNeeded--;
                if (deadAgent.backspacesNeeded <= 0)
                {
                    mAgents.emplace_back(deadAgent);
                    return true;
                }
                return false;
            }
        );
        return;
    }

    // returns true if an ending was found
    const auto lambdaAdvanceAgent = [this, length, inputs](const Agent& agent, wchar_t inputLetter, bool isBeingComposed, int inputIndex)
        {
            if (mIsTriggerTreeOutdated.load())
            {
                // Pretend a trigger was found so that the invalid agents are cleared & short-circuit the remaining checks.
                return true;
            }

            const Node& node = *agent.node;
            // TODO: Maybe use binary search(std::equal_range)? Should modify the spaceship operator too, then.

            bool didFindMatchingChild = false;
            for (int childIndex = node.childStartIndex; childIndex < node.childStartIndex + node.childLength; childIndex++)
            {
                const Node& child = mTree.at(childIndex);
                if (child.letter != inputLetter)
                {
                    continue;
                }

                Agent nextAgent{ .node = &child, .strokeStartIndex = agent.strokeStartIndex - 1 };
                if (child.endingIndex < 0)
                {
                    // If the letter is being composed, only check for the triggers, don't advance the agents.
                    // ex - Typing '갃' should match '가' in the middle of the composition.
                    // But we should not advance the agents since doing so would fail to match any Korean letters which are composed more than 1 letter.
                    if (!isBeingComposed)
                    {
                        mNextIterationAgents.emplace_back(nextAgent);
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

                replaceString(mEndings.at(child.endingIndex), nextAgent, mStroke, inputs, length, inputIndex, doNeedFullComposite);

                return true;
            }

            if (!didFindMatchingChild && agent != mRootAgent)
            {
                mDeadAgents.emplace_back(agent);
            }
            return false;
        };

    for (int i = 0; i < length; i++)
    {
        const auto [inputLetter, isBeingComposed] = inputs[i];

        if (!isBeingComposed)
        {
            std::ranges::shift_left(mStroke, 1);
            mStroke.back() = inputLetter;
        }

        // Check for triggers in the root node first.
        bool isTriggerFound = lambdaAdvanceAgent(mRootAgent, inputLetter, isBeingComposed, i);
        if (!isTriggerFound)
        {
            for (const Agent& agent : mAgents)
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
            mNextIterationAgents.clear();
            mDeadAgents.clear();
        }

        if (!isBeingComposed)
        {
            std::erase_if(mDeadAgents, [](DeadAgent& deadAgent) { return ++deadAgent.backspacesNeeded > get_config().maxBackspaceCount; });
        }

        if (!isBeingComposed || isTriggerFound)
        {
            mAgents.clear();
            std::swap(mAgents, mNextIterationAgents);
        }
    }
}


void TriggerTree::replaceString(const Ending& ending, const Agent& agent, std::wstring_view stroke, 
    const InputMessage(&inputs)[MAX_INPUT_COUNT], int inputLength, int inputIndex, bool doNeedFullComposite)
{
    const auto& [replaceStringIndex, replaceType, replaceStringLength, backspaceCount, cursorMoveCount,
        propagateCase, uppercaseStyle, keepComposite] = ending;

    const std::wstring_view originalReplaceString{ mReplaceStrings.data() + replaceStringIndex, replaceStringLength };

    imm_simulator.ClearComposition();

    switch (replaceType)
    {
    case Ending::EReplaceType::IMAGE:
    {
        push_current_clipboard_state();
        std::vector<FakeInput> fakeInputs{ backspaceCount, FakeInput{ FakeInput::EType::KEY, FakeInput::BACKSPACE_KEY } };
        if (set_clipboard_image(originalReplaceString))
        {
            // Popping the clipboard state is done in main.
            fakeInputs.emplace_back(FakeInput::EType::HOT_KEY_PASTE);
        }
        else
        {
            pop_clipboard_state();
        }
        send_fake_inputs(fakeInputs, false);
        return;
    }

    case Ending::EReplaceType::COMMAND:
    {
        std::vector<FakeInput> fakeInputs{ backspaceCount, FakeInput{ FakeInput::EType::KEY, FakeInput::BACKSPACE_KEY } };
        const auto& [str, ret] = run_command_and_get_output(originalReplaceString);
        fakeInputs.reserve(fakeInputs.size() + str.size());
        for (wchar_t c : str)
        {
            fakeInputs.emplace_back(FakeInput::EType::LETTER, c);
        }
        send_fake_inputs(fakeInputs, false);
        return;
    }

    case Ending::EReplaceType::TEXT:
        break;

    default:
        std::unreachable();
    }

    std::wstring replaceString{ originalReplaceString };

    unsigned int additionalCursorMoveCount = 0;

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
            const std::wstring lastLetterNormalized = decompose_hangeul(lastLetterString);
            lastLetterString = hangeul_to_alphabet(lastLetterNormalized, false);
            // The length of the middle letters + the last letter's decomposition.
            additionalBackspaceCount += static_cast<unsigned int>(lastLetterNormalized.size()) - 1;
            for (const wchar_t ch : lastLetterNormalized)
            {
                imm_simulator.AddLetter(ch, false);
            }
        }
        const bool shouldToggleHangeul = isLastLetterKorean && !is_hangeul_on;

        // Note that we're not using the backspaceCount from the ending,
        // since the last letter of the replace string was decomposed to calculate the count (we don't want that here).
        const unsigned int totalBackspaceCount = mTreeHeight - agent.strokeStartIndex + additionalBackspaceCount;
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
        const std::wstring lastLetterNormalized = decompose_hangeul(replace.substr(replaceStringLength - 1));
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
            imm_simulator.AddLetter(ch, false);
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

    for (FakeInput& fakeInput : fakeInputs)
    {
        if (fakeInput.type == FakeInput::EType::LETTER && fakeInput.letter == '\n')
        {
            fakeInput = { FakeInput::EType::KEY, FakeInput::ENTER_KEY };
        }
    }

    send_fake_inputs(fakeInputs, false);
}
