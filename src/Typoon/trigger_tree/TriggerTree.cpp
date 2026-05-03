#include "TriggerTree.h"

#include <deque>
#include <map>
#include <optional>
#include <queue>
#include <ranges>
#include <memory>

#include "Letter.h"
#include "util/Hangeul.h"
#include "util/String.h"


namespace
{
    struct TempEnding
    {
        std::wstring_view replace;
        typoon::core::EReplaceType type = typoon::core::EReplaceType::TEXT;

        unsigned int backspaceCount = 0;
        unsigned int cursorMoveCount = 0;

        bool doPropagateCase = false;
        typoon::core::EUppercaseStyle uppercaseStyle = typoon::core::EUppercaseStyle::FIRST_LETTER;
        bool doKeepComposite = false;
    };

    struct TempNode
    {
        std::map<typoon::core::Letter, std::unique_ptr<TempNode>> children{};
        TempEnding ending{};

        /// 첫 iteration에서 사용
        const typoon::core::Match* match = nullptr;
        std::wstring_view trigger{};

        /// 두 번째 iteration에서 사용
        int parentIndex = -1;
        const typoon::core::Letter* letter = nullptr;
        unsigned int height = 0;

        [[nodiscard]] bool IsEndNode() const { return children.empty(); }
    };


    std::vector<std::wstring> TryModifyTriggers(
        const typoon::core::Match& match, 
        const typoon::core::Options& options)
    {
        std::vector<std::wstring> modifiedTriggers;

        if (!options.isKorEngInsensitive && !options.needWord)
        {
            return modifiedTriggers;
        }

        modifiedTriggers.reserve(match.triggers.size() * (options.isKorEngInsensitive ? 2 : 1));

        for (const std::wstring& trigger : match.triggers)
        {
            if (options.isKorEngInsensitive)
            {
                // 한글/라틴 알파벳이 섞여 있을 수 있으므로 양방향 변환
                modifiedTriggers.emplace_back(
                    typoon::util::combine_hangeul(typoon::util::latin_alphabet_to_hangeul_alphabet(trigger)) +
                    (options.needWord ? typoon::core::Letter::NON_WORD_LETTER : wchar_t{})
                );
                modifiedTriggers.emplace_back(
                    typoon::util::hangeul_alphabet_to_latin_alphabet(typoon::util::decompose_hangeul(trigger)) +
                    (options.needWord ? typoon::core::Letter::NON_WORD_LETTER : wchar_t{})
                );
            }
            else if (options.needWord)
            {
                modifiedTriggers.emplace_back(trigger + typoon::core::Letter::NON_WORD_LETTER);
            }
        }

        return modifiedTriggers;
    }

    std::pair<std::wstring, unsigned int> TryModifyReplaceAndGetCursorMoveCount(
        const typoon::core::Match& match, 
        const typoon::core::Options& options, 
        const std::wstring_view cursorPlaceholder)
    {
        std::wstring modifiedReplace;
        unsigned int cursorMoveCount = 0;

        if (options.needWord)
        {
            modifiedReplace = match.replace + typoon::core::Letter::LAST_INPUT_LETTER;
        }

        // 이 위에서 replace에 영향을 주는 건 word가 맨 끝에 LAST_INPUT_LETTER를 추가하는 것밖에 없으므로 match.replace에서 찾아도 됨
        if (const size_t cursorIndex = match.replace.find(cursorPlaceholder);
            cursorIndex != std::wstring::npos)
        {
            if (modifiedReplace.empty())
            {
                modifiedReplace = match.replace;
            }
            modifiedReplace.erase(cursorIndex, cursorPlaceholder.size());

            cursorMoveCount = static_cast<unsigned int>(modifiedReplace.size() - cursorIndex);
        }

        return { std::move(modifiedReplace), cursorMoveCount };
    }

    std::optional<std::pair<const typoon::core::Match*, std::wstring_view>> GetOverridingTrigger(
        const std::wstring_view trigger, 
        const bool isCaseSensitive, 
        const TempNode& root)
    {
        // 기본적으로 확인해야 할 사항은 이 트리거의 substring이 또 다른 트리거로 존재하는지이다.
        // 조금 복잡하게 만드는 요소는 이 트리거의 각 글자마다 해당 Letter가 트리거될 때 똑같이 트리거될 다른 모든 Letter들에 대해 확인해야 한단 것이다.
        // 예) 기존 트리거: ple (case-insensitive) / 새 트리거: APPLE (case-sensitive)
        // 기존 트리거는 ple, PLE에 대해 모두 트리거되므로 새 트리거의 PLE를 가린다. 즉, P(c-s)를 추가할 때 p(c-i)까지 확인해야 한다.

        std::vector<const TempNode*> agents;
        std::vector<const TempNode*> nextAgents;

        for (const wchar_t c : trigger)
        {
            for (const typoon::core::Letter& letterToCheck : typoon::core::Letter{ c, isCaseSensitive }.GetSupersetLetters())
            {
                if (const auto it = root.children.find(letterToCheck);
                    it != root.children.end())
                {
                    nextAgents.emplace_back(it->second.get());
                }

                for (const TempNode* agent : agents)
                {
                    if (const auto it = agent->children.find(letterToCheck);
                        it != agent->children.end())
                    {
                        nextAgents.emplace_back(it->second.get());
                    }
                }
            }

            agents.clear();
            std::swap(agents, nextAgents);

            for (const TempNode* agent : agents)
            {
                if (!agent->IsEndNode())
                {
                    continue;
                }

                return std::make_pair(agent->match, agent->trigger);
            }
        }

        return std::nullopt;
    }

    std::vector<std::pair<const typoon::core::Match*, std::wstring_view>> GetOverridenTriggers(
        const std::wstring_view trigger, 
        const bool isCaseSensitive, 
        const std::map<typoon::core::Letter, std::deque<const TempNode*>>& nodesPerLetter)
    {
        // 복잡한 이유가 위와 조금 다른데, 이 트리거의 Letter가 완벽히 가리는 경우들을 포함해야 한다. 즉 역방향이다.
        // 당연한 것이, 확인해야 할 상황도 정반대이다. 이 트리거가 또 다른 트리거의 substring인지 확인해야 한다.

        std::vector<const TempNode*> agents;
        std::vector<const TempNode*> nextAgents;

        for (const typoon::core::Letter& letterToCheck : typoon::core::Letter{ trigger.front(), isCaseSensitive }.GetSubsetLetters())
        {
            if (const auto it = nodesPerLetter.find(letterToCheck);
                it != nodesPerLetter.end())
            {
                agents.append_range(it->second);
            }
        }

        if (agents.empty())
        {
            return {};
        }

        for (const wchar_t c : trigger.substr(1))
        {
            for (const typoon::core::Letter& letterToCheck : typoon::core::Letter{ c, isCaseSensitive }.GetSubsetLetters())
            {
                for (const TempNode* agent : agents)
                {
                    if (const auto it = agent->children.find(letterToCheck);
                        it != agent->children.end())
                    {
                        nextAgents.emplace_back(it->second.get());
                    }
                }
            }

            agents.clear();
            std::swap(agents, nextAgents);

            if (agents.empty())
            {
                return {};
            }
        }

        std::vector<std::pair<const typoon::core::Match*, std::wstring_view>> result;
        result.reserve(agents.size());
        for (const TempNode* agent : agents)
        {
            result.emplace_back(std::make_pair(agent->match, agent->trigger));
        }
        return result;
    }
}


namespace typoon::core
{
    TriggerTree::TriggerTree(
        const std::span<const core::Match> matches, 
        const std::wstring_view cursorPlaceholder,
        const util::NullableCallback<std::span<const TriggerTreeBuildError>>& errorCallback)
    {
        // 첫 iteration에서 생성 후 두 번째 iteration까지 string_view로 참조하므로 두 번째 loop를 outlive하고 메모리상 고정돼야 함
        struct ModifiedStringStorage
        {
            std::vector<std::wstring> modifiedTriggers;  // 한 번 채워진 후 변경되지 않으므로 재할당이 일어나도 괜찮음
            std::wstring modifiedReplace;
        };
        std::vector<ModifiedStringStorage> modifiedStringStorages;
        modifiedStringStorages.resize(matches.size());

        std::vector<TriggerTreeBuildError> buildErrors;

        TempNode root{};
        std::map<Letter, std::deque<const TempNode*>> nodesPerLetter;

        // 첫 iteration. 실제로 사용될 트리거와 대치 텍스트를 생성하고, tree를 건설하며 도달 불가능한 트리거를 솎아냄
        const int matchCount = static_cast<int>(matches.size());
        for (int i = 0; i < matchCount; i++)
        {
            const core::Match& match = matches[i];
            ModifiedStringStorage& modifiedStringStorage = modifiedStringStorages.at(i);

            const core::Options& options = match.options;

            modifiedStringStorage.modifiedTriggers = TryModifyTriggers(match, options);
            unsigned int cursorMoveCount;
            std::tie(modifiedStringStorage.modifiedReplace, cursorMoveCount) = TryModifyReplaceAndGetCursorMoveCount(match, options, cursorPlaceholder);

            std::span<const std::wstring> triggersToUse = modifiedStringStorage.modifiedTriggers.empty() ? match.triggers : modifiedStringStorage.modifiedTriggers;
            std::wstring_view replaceToUse = modifiedStringStorage.modifiedReplace.empty() ? match.replace : modifiedStringStorage.modifiedReplace;

            for (const std::wstring_view trigger : triggersToUse)
            {
                // 1. 이 트리거가 도달 가능한지 확인
                if (const auto overridingTrigger = GetOverridingTrigger(trigger, options.isCaseSensitive, root))
                {
                    const auto [otherMatch, otherTrigger] = overridingTrigger.value();
                    buildErrors.emplace_back(ETriggerTreeBuildErrorType::UNREACHABLE_TRIGGER, &match, trigger, otherMatch, otherTrigger);
                    continue;
                }

                // 2. 이 트리거로 인해 도달 불가능하게 될 트리거들을 확인
                if (const auto overridenTriggers = GetOverridenTriggers(trigger, options.isCaseSensitive, nodesPerLetter);
                    !overridenTriggers.empty())
                {
                    for (const auto [otherMatch, otherTrigger] : overridenTriggers)
                    {
                        buildErrors.emplace_back(ETriggerTreeBuildErrorType::UNREACHABLE_TRIGGER, otherMatch, otherTrigger, &match, trigger);
                    }

                    // 굳이 도달 불가능해진 트리거들의 노드를 지우려고 하진 않는다.
                    // 다른 도달 가능한 트리거에 쓰이는지 확인도 귀찮을뿐더러 웬만하면 매치 파일을 수정하고 다시 빌드할 것이기 때문.
                }

                // 3. 트리거를 트리에 삽입

                // 마지막 문자가 한글일 경우 지우는 데 백스페이스를 여러 번 눌러야 할 수 있음
                const wchar_t triggerLastLetter = trigger.back();
                auto backspaceCount = static_cast<unsigned int>(trigger.size());
                if (util::is_hangeul(triggerLastLetter))
                {
                    const std::wstring lastLetterDecomposed = util::decompose_hangeul(std::wstring_view{ &triggerLastLetter, 1 });
                    backspaceCount += static_cast<int>(lastLetterDecomposed.size()) - 1;
                }

                const TempEnding tempEnding{
                    .replace = replaceToUse,
                    .type = match.replaceType,

                    .backspaceCount = backspaceCount,
                    .cursorMoveCount = cursorMoveCount,

                    .doPropagateCase = options.doPropagateCase,
                    .uppercaseStyle = options.uppercaseStyle,
                    .doKeepComposite = options.doKeepComposite,
                };

                TempNode endNode{
                    .ending = tempEnding,
                    .match = &match,
                    .trigger = trigger,
                };

                TempNode* currentNode = &root;
                for (const wchar_t c : trigger.substr(0, trigger.size() - 1))
                {
                    const Letter letter{ c, options.isCaseSensitive };
                    if (!currentNode->children.contains(letter))
                    {
                        TempNode node{
                            .match = &match,
                            .trigger = trigger
                        };

                        auto newNode = std::make_unique<TempNode>(std::move(node));
                        nodesPerLetter[letter].emplace_back(newNode.get());

                        currentNode->children[letter] = std::move(newNode);
                    }

                    currentNode = currentNode->children.at(letter).get();
                }

                const Letter lastLetter{ trigger.back(), options.isCaseSensitive, options.needFullComposite };

                auto newNode = std::make_unique<TempNode>(std::move(endNode));
                nodesPerLetter[lastLetter].emplace_back(newNode.get());

                if (const auto it = currentNode->children.find(lastLetter);
                    it != currentNode->children.end())
                {
                    std::erase(nodesPerLetter[lastLetter], it->second.get());
                }
                currentNode->children[lastLetter] = std::move(newNode);
            }
        }

        if (!buildErrors.empty())
        {
            errorCallback(buildErrors);
        }

        std::queue<TempNode*> nodes;
        nodes.push(&root);

        // 두 번째 iteration. 캐시 효율을 최대화하기 위해 tree를 flatten하고 대치 문자열을 하나의 문자열로 합침
        // 한 노드의 모든 자식들이 contiguous하도록 level-order로 traverse
        while (!nodes.empty())
        {
            TempNode* tempNode = nodes.front();
            nodes.pop();

            Node newNode{
                .parentIndex = tempNode->parentIndex
            };
            if (tempNode->letter)
            {
                newNode.letter = *tempNode->letter;
            }

            if (tempNode->IsEndNode())
            {
                newNode.endingIndex = static_cast<int>(endings.size());

                const TempEnding& tempEnding = tempNode->ending;

                // 중복 감지 로직 개선은 하지 않을 예정. Shortest Common SuperString Problem은 NP-hard problem이기 때문에 애초에 최적의 결과를 도출할 수도 없고,
                // 메모리 사용량은 우선순위가 아니기 때문.
                int replaceStringIndex = -1;
                if (const size_t result = compiledReplaceStrings.find(tempEnding.replace);
                    result == std::wstring::npos)
                {
                    replaceStringIndex = static_cast<int>(compiledReplaceStrings.size());
                    compiledReplaceStrings.append(tempEnding.replace);
                }
                else
                {
                    replaceStringIndex = static_cast<int>(result);
                }

                Ending ending{
                    .type = tempEnding.type,
                    .replaceStringIndex = replaceStringIndex,
                    .replaceStringLength = static_cast<unsigned int>(tempEnding.replace.size()),
                    .backspaceCount = tempEnding.backspaceCount,
                    .cursorMoveCount = tempEnding.cursorMoveCount,
                    .doPropagateCase = tempEnding.doPropagateCase,
                    .uppercaseStyle = tempEnding.uppercaseStyle,
                    .doKeepComposite = tempEnding.doKeepComposite,
                };
                endings.emplace_back(ending);
            }

            const int index = static_cast<int>(flattenedTree.size());
            flattenedTree.emplace_back(newNode);

            if (tempNode->parentIndex >= 0)
            {
                Node& parent = flattenedTree.at(tempNode->parentIndex);
                if (parent.childStartIndex < 0)
                {
                    parent.childStartIndex = index;
                }
                if (tempNode->letter->IsSpecial())
                {
                    parent.specialChildCount++;
                }
                parent.childCount++;
            }

            treeHeight = std::max(treeHeight, tempNode->height);

            // 자식들에게 정보를 미리 저장
            for (auto& [letter, child] : tempNode->children)
            {
                child->parentIndex = index;
                child->letter = &letter;
                child->height = tempNode->height + 1;
                nodes.push(child.get());
            }
        }
    }
}
