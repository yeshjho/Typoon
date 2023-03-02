#include "trigger_tree.h"

#include <cwctype>
#include <map>

#include "../utils/string.h"


void construct_trigger_tree(const std::vector<Match>& matches)
{
    // First iteration. Focus on 'constructing' the tree, runtime efficiency will be handled later.
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
        bool isTrigger;
        std::wstring replace;
    };

    Node root{ .isTrigger = false };

    for (const auto& [triggers, replace, isCaseSensitive] : matches)
    {
        for (const auto& trigger : triggers)
        {
            Node* node = &root;
            for (auto triggerIt = trigger.begin(); triggerIt != trigger.end() - 1; ++triggerIt)
            {
                const wchar_t ch = *triggerIt;
                auto [it, isNew] = node->children.try_emplace(Letter{ ch, isCaseSensitive && is_cased_alpha(ch) }, Node{ .isTrigger = false });
                if (!isNew && it->second.isTrigger)  // Same case with the last letter overwriting, but in a reverse order.
                {
                    break;
                }
                node = &it->second;
            }

            // Since finding a match resets all the agents, we cannot advance further anyway. Therefore overwriting is fine.
            node->children[{ trigger.back(), isCaseSensitive && is_cased_alpha(trigger.back()) }] = { .isTrigger = true, .replace = replace };
        }
    }
}
