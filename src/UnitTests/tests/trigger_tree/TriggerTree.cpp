#include <doctest.h>

#include <algorithm>

#include "Typoon/trigger_tree/TriggerTree.h"

#include "util/DocTestWStringSupport.h"


using typoon::core::TriggerTree;
using typoon::core::Ending;
using typoon::core::Letter;
using typoon::core::Match;
using typoon::core::Node;
using typoon::core::TriggerTreeBuildError;

namespace
{
    constexpr std::wstring_view CURSOR_PLACEHOLDER = L"|_|";
}


TEST_SUITE("TriggerTree")
{
    TEST_CASE("기본 건설")
    {
        SUBCASE("기초")
        {
            const Match matches[] {
                Match{
                    .triggers = { L"abzx", L"abcde" },
                    .replace = L"def"
                },
            };

            const auto lambdaOnError = [](std::span<const TriggerTreeBuildError> /*error*/) { CHECK(false); };

            const TriggerTree tt{ matches, CURSOR_PLACEHOLDER, lambdaOnError };

            const Node expectedTree[] = {
                Node{
                    .letter = Letter{},
                    .parentIndex = -1,
                    .childStartIndex = 1,
                    .specialChildCount = 0,
                    .childCount = 1,
                    .endingIndex = -1,
                },
                Node{
                    .letter = Letter{ L'a' },
                    .parentIndex = 0,
                    .childStartIndex = 2,
                    .specialChildCount = 0,
                    .childCount = 1,
                    .endingIndex = -1,
                },
                Node{
                    .letter = Letter{ L'b' },
                    .parentIndex = 1,
                    .childStartIndex = 3,
                    .specialChildCount = 0,
                    .childCount = 2,
                    .endingIndex = -1,
                },
                Node{
                    .letter = Letter{ L'c' },
                    .parentIndex = 2,
                    .childStartIndex = 5,
                    .specialChildCount = 0,
                    .childCount = 1,
                    .endingIndex = -1,
                },
                Node{
                    .letter = Letter{ L'z' },
                    .parentIndex = 2,
                    .childStartIndex = 6,
                    .specialChildCount = 0,
                    .childCount = 1,
                    .endingIndex = -1,
                },
                Node{
                    .letter = Letter{ L'd' },
                    .parentIndex = 3,
                    .childStartIndex = 7,
                    .specialChildCount = 0,
                    .childCount = 1,
                    .endingIndex = -1,
                },
                Node{
                    .letter = Letter{ L'x' },
                    .parentIndex = 4,
                    .childStartIndex = -1,
                    .specialChildCount = 0,
                    .childCount = 0,
                    .endingIndex = 0,
                },
                Node{
                    .letter = Letter{ L'e' },
                    .parentIndex = 5,
                    .childStartIndex = -1,
                    .specialChildCount = 0,
                    .childCount = 0,
                    .endingIndex = 1,
                },
            };

            const Ending expectedEndings[] = {
                Ending{
                    .replaceStringIndex = 0,
                    .replaceStringLength = 3,
                    .backspaceCount = 4,
                },
                Ending{
                    .replaceStringIndex = 0,
                    .replaceStringLength = 3,
                    .backspaceCount = 5,
                }
            };

            
            CHECK(std::ranges::equal(tt.flattenedTree, expectedTree));
            CHECK(tt.treeHeight == 5);
            CHECK(std::ranges::equal(tt.endings, expectedEndings));
            CHECK(tt.compiledReplaceStrings == L"def");
        }
    }

    TEST_CASE("")
    {
        SUBCASE("")
        {
            // 예) sensitive: abcd
            //   insensitive: abc
            // type: abc -> abc / type: ABC -> abc니까 공존 불가능
            // 
            // 예) sensitive: abcd
            //   insensitive: abcde
            // type: ABCDE -> abcde / type: abcd -> abcd니까 공존 가능
        }
    }
}