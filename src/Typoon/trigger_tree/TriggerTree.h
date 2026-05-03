#pragma once
#include <span>

#include "Letter.h"
#include "Match.h"
#include "TriggerTreeBuildError.h"
#include "util/Function.h"


namespace typoon::core
{
    struct Node
    {
        Letter letter{};

        int parentIndex = -1;

        int childStartIndex = -1;
        int specialChildCount = 0;
        int childCount = 0;

        int endingIndex = -1;

        [[nodiscard]] bool operator==(const Node&) const = default;
    };

    struct NodeComp
    {
        [[nodiscard]] bool operator()(const Node& node, const wchar_t ch) const { return node.letter < ch; }
        [[nodiscard]] bool operator()(const wchar_t ch, const Node& node) const { return ch < node.letter; }
    };

    struct Ending
    {
        EReplaceType type = EReplaceType::TEXT;

        int replaceStringIndex = -1;
        unsigned int replaceStringLength = 0;

        unsigned int backspaceCount = 0;
        unsigned int cursorMoveCount = 0;

        bool doPropagateCase = false;
        EUppercaseStyle uppercaseStyle = EUppercaseStyle::FIRST_LETTER;
        bool doKeepComposite = false;

        [[nodiscard]] bool operator==(const Ending&) const = default;
    };

    /**
     * @brief 모든 매치의 트리거를 모아 트리를 구성, leaf 노드에 도달 시 해당하는 replace(Ending)가 있음
     */
    struct TriggerTree
    {
        explicit TriggerTree(
            std::span<const core::Match> matches, 
            std::wstring_view cursorPlaceholder, 
            const util::NullableCallback<std::span<const TriggerTreeBuildError>>& errorCallback = nullptr);


        std::vector<Node> flattenedTree{};
        unsigned int treeHeight = 0;
        std::vector<Ending> endings{};
        std::wstring compiledReplaceStrings{};
    };
}
