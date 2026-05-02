#include "TriggerTree.h"

#include <map>
#include <queue>
#include <ranges>

#include "Letter.h"
#include "util/Hangeul.h"


namespace typoon::core
{
    struct TempEnding
    {
        std::wstring_view replace;
        EReplaceType type = EReplaceType::TEXT;

        unsigned int backspaceCount = 0;
        unsigned int cursorMoveCount = 0;

        bool doPropagateCase = false;
        EUppercaseStyle uppercaseStyle = EUppercaseStyle::FIRST_LETTER;
        bool doKeepComposite = false;
    };

    struct TempNode
    {
        std::map<Letter, TempNode> children{};
        TempEnding ending{};

        /// 첫 iteration에서 사용
        const core::Match* match = nullptr;
        std::wstring_view trigger{};

        /// 두 번째 iteration에서 사용
        int parentIndex = -1;
        const Letter* letter = nullptr;
        unsigned int height = 0;

        [[nodiscard]] bool IsEndNode() const { return children.empty(); }
    };


    TriggerTree::TriggerTree(const std::span<const core::Match> matches, const std::wstring_view cursorPlaceholder,
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

        // 첫 iteration. 실제로 사용될 트리거와 대치 텍스트를 생성하고, tree를 건설하며 도달 불가능한 트리거를 솎아냄
        const int matchCount = static_cast<int>(matches.size());
        for (int i = 0; i < matchCount; i++)
        {
            const core::Match& match = matches[i];
            ModifiedStringStorage& modifiedStringStorage = modifiedStringStorages.at(i);

            const core::Options& options = match.options;

            modifiedStringStorage.modifiedTriggers = tryModifyTriggers(match, options);
            unsigned int cursorMoveCount;
            std::tie(modifiedStringStorage.modifiedReplace, cursorMoveCount) = tryModifyReplaceAndGetCursorMoveCount(match, options, cursorPlaceholder);

            std::span<const std::wstring> triggersToUse = modifiedStringStorage.modifiedTriggers.empty() ? match.triggers : modifiedStringStorage.modifiedTriggers;
            std::wstring_view replaceToUse = modifiedStringStorage.modifiedReplace.empty() ? match.replace : modifiedStringStorage.modifiedReplace;

            for (const std::wstring_view trigger : triggersToUse)
            {
                TempNode* currentNode = &root;

                bool isUnreachable = false;
                for (const wchar_t c : trigger | std::views::take(trigger.size() - 1))
                {
                    Letter letter{ c, options.isCaseSensitive, false };
                    TempNode newNode{
                        .match = &match,
                        .trigger = trigger
                    };
                    auto [it, isNew] = currentNode->children.try_emplace(letter, std::move(newNode));
                    auto& [_, node] = *it;

                    // 동일한 글자가 존재하는데 그 글자가 한 트리거의 마지막 문자라면 이 트리거는 도달 불가능
                    if (!isNew && node.IsEndNode())
                    {
                        buildErrors.emplace_back(TriggerTreeBuildError{
                            .type = ETriggerTreeBuildErrorType::UNREACHABLE_TRIGGER,
                            .match = &match,
                            .trigger = trigger,
                            .otherMatch = currentNode->match,
                            .otherTrigger = currentNode->trigger,
                        });
                        isUnreachable = true;
                        break;
                    }

                    currentNode = &node;
                }
                if (isUnreachable)
                {
                    continue;
                }

                Letter lastLetter{ trigger.back(), options.isCaseSensitive, options.needFullComposite };
                if (const auto it = currentNode->children.find(lastLetter);
                    it != currentNode->children.end())
                {
                    auto& [_, node] = *it;

                    // 이미 다른 end node가 존재하는 경우 기존 것을 유지 (두 트리거가 완전 동일한 경우임)
                    if (node.IsEndNode())
                    {
                        buildErrors.emplace_back(TriggerTreeBuildError{
                            .type = ETriggerTreeBuildErrorType::IDENTICAL_TRIGGER,
                            .match = &match,
                            .trigger = trigger,
                            .otherMatch = currentNode->match,
                            .otherTrigger = currentNode->trigger,
                        });
                        continue;
                    }

                    // end node가 아닐 경우 다른 트리거들이 이 트리거에 의해 도달하지 못하게 된다는 뜻이므로 traverse하며 build error에 추가
                    std::queue<const TempNode*> staleNodes;
                    staleNodes.push(&node);
                    while (!staleNodes.empty())
                    {
                        const TempNode* childNode = staleNodes.front();
                        staleNodes.pop();

                        if (childNode->IsEndNode())
                        {
                            buildErrors.emplace_back(TriggerTreeBuildError{
                                .type = ETriggerTreeBuildErrorType::UNREACHABLE_TRIGGER,
                                .match = childNode->match,
                                .trigger = childNode->trigger,
                                .otherMatch = &match,
                                .otherTrigger = trigger,
                            });
                        }
                        else
                        {
                            for (const TempNode& child : childNode->children | std::views::values)
                            {
                                staleNodes.push(&child);
                            }
                        }
                    }
                }

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
                currentNode->children[lastLetter] = std::move(endNode);
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
                child.parentIndex = index;
                child.letter = &letter;
                child.height = tempNode->height + 1;
                nodes.push(&child);
            }
        }
    }

    std::vector<std::wstring> TriggerTree::tryModifyTriggers(const core::Match& match, const core::Options& options)
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
                    util::combine_hangeul(util::latin_alphabet_to_hangeul_alphabet(trigger)) +
                    (options.needWord ? Letter::NON_WORD_LETTER : wchar_t{})
                );
                modifiedTriggers.emplace_back(
                    util::hangeul_alphabet_to_latin_alphabet(util::decompose_hangeul(trigger)) +
                    (options.needWord ? Letter::NON_WORD_LETTER : wchar_t{})
                );
            }
            else if (options.needWord)
            {
                modifiedTriggers.emplace_back(trigger + Letter::NON_WORD_LETTER);
            }
        }

        return modifiedTriggers;
    }

    std::pair<std::wstring, unsigned int> TriggerTree::tryModifyReplaceAndGetCursorMoveCount(const core::Match& match, const core::Options& options, const std::wstring_view cursorPlaceholder)
    {
        std::wstring modifiedReplace;
        unsigned int cursorMoveCount = 0;

        if (options.needWord)
        {
            modifiedReplace = match.replace + Letter::LAST_INPUT_LETTER;
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
}
