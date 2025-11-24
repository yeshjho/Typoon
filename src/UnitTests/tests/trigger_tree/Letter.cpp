#include <doctest.h>

#include <algorithm>
#include <random>

#include "Typoon/trigger_tree/Letter.h"

#include "util/DocTestWStringSupport.h"


using typoon::core::Letter;

TEST_SUITE("Letter")
{
    TEST_CASE("operator==(wchar_t ch)")
    {
        SUBCASE("라틴 문자 - 대소문자 구별 안 함")
        {
            {
                const Letter letter{ L'a', false };

                CHECK(letter == L'a');
                CHECK(letter == L'A');
                CHECK(letter != L'b');
                CHECK(letter != L'B');
                CHECK(letter != L'1');
                CHECK(letter != L'.');
                CHECK(letter != L'ㅁ');
                CHECK(letter != L'á');
                CHECK(letter != L'Á');
            }
            {
                const Letter letter{ L'O', false };

                CHECK(letter == L'o');
                CHECK(letter == L'O');
                CHECK(letter != L'p');
                CHECK(letter != L'P');
                CHECK(letter != L'0');
                CHECK(letter != L'.');
                CHECK(letter != L'ㅐ');
                CHECK(letter != L'ㅒ');
                CHECK(letter != L'ǒ');
                CHECK(letter != L'ø');
            }
        }

        SUBCASE("라틴 문자 - 대소문자 구별함")
        {
            {
                const Letter letter{ L'a', true };

                CHECK(letter == L'a');
                CHECK(letter != L'A');
                CHECK(letter != L'b');
                CHECK(letter != L'B');
                CHECK(letter != L'1');
                CHECK(letter != L'.');
                CHECK(letter != L'ㅁ');
                CHECK(letter != L'á');
                CHECK(letter != L'Á');
            }
            {
                const Letter letter{ L'O', true };

                CHECK(letter != L'o');
                CHECK(letter == L'O');
                CHECK(letter != L'p');
                CHECK(letter != L'P');
                CHECK(letter != L'0');
                CHECK(letter != L'.');
                CHECK(letter != L'ㅐ');
                CHECK(letter != L'ㅒ');
                CHECK(letter != L'ǒ');
                CHECK(letter != L'ø');
            }
        }

        SUBCASE("한글 문자")
        {
            {
                const Letter letter{ L'ㄱ' };

                CHECK(letter == L'ㄱ');
                CHECK(letter != L'ᄀ');  // 초성(U+1100)
                CHECK(letter != L'ᆨ');  // 종성(U+11A8)
                CHECK(letter != L'ㄲ');
                CHECK(letter != L'ㄴ');
                CHECK(letter != L'r');
                CHECK(letter != L'R');
                CHECK(letter != L'1');
                CHECK(letter != L'.');
            }
            {
                const Letter letter{ L'가' };

                CHECK(letter == L'가');
                CHECK(letter != L'각');
                CHECK(letter != L'나');
                CHECK(letter != L'r');
                CHECK(letter != L'R');
                CHECK(letter != L'1');
                CHECK(letter != L'.');
            }
            {
                const Letter letter{ L'쒡' };

                CHECK(letter == L'쒡');
                CHECK(letter != L'쒠');
                CHECK(letter != L'쒜');
                CHECK(letter != L'쑤');
                CHECK(letter != L'ㅆ');
                CHECK(letter != L'쒢');
                CHECK(letter != L't');
                CHECK(letter != L'T');
                CHECK(letter != L'1');
                CHECK(letter != L'.');
            }
        }

        SUBCASE("특수 문자")
        {
            {
                const Letter letter{ Letter::NON_WORD_LETTER };

                CHECK(letter == L' ');
                CHECK(letter == L'\n');
                CHECK(letter == L'\t');
                CHECK(letter == L'.');
                CHECK(letter == L';');
                CHECK(letter == L'!');
                CHECK(letter == L'?');
                CHECK(letter == L'"');
                CHECK(letter == L')');
                CHECK(letter == L'-');
                CHECK(letter != L'1');
                CHECK(letter != L'5');
                CHECK(letter != L'a');
                CHECK(letter != L'P');
                CHECK(letter != L'ㄱ');
                CHECK(letter != L'닭');
                CHECK(letter != L'ㅕ');
                CHECK(letter != L'ㄳ');
            }
        }

        SUBCASE("doNeedFullComposite은 아무 영향 없음")
        {
            {
                const Letter letter{ L'ㄱ', false, true };

                CHECK(letter == L'ㄱ');
                CHECK(letter != L'r');
                CHECK(letter != L'R');
            }
            {
                const Letter letter{ L'가', false, true };

                CHECK(letter == L'가');
                CHECK(letter != L'r');
                CHECK(letter != L'R');
            }
            {
                const Letter letter{ L'쒡', false, true };

                CHECK(letter == L'쒡');
                CHECK(letter != L'쒠');
                CHECK(letter != L'쒜');
                CHECK(letter != L'쑤');
                CHECK(letter != L'ㅆ');
                CHECK(letter != L'쒢');
                CHECK(letter != L't');
                CHECK(letter != L'T');
                CHECK(letter != L'1');
                CHECK(letter != L'.');
            }
        }
    }

    TEST_CASE("operator==(const Letter& other)")
    {
        SUBCASE("라틴 문자 - 대소문자 구별 안 함 vs 대소문자 구별 안 함")
        {
            {
                const Letter letter{ L'a', false };

                CHECK(letter == Letter{ L'a', false });
                CHECK(letter == Letter{ L'A', false });
                CHECK(letter != Letter{ L'b', false });
                CHECK(letter != Letter{ L'B', false });
                CHECK(letter != Letter{ L'1', false });
                CHECK(letter != Letter{ L'.', false });
                CHECK(letter != Letter{ L'ㅁ', false });
                CHECK(letter != Letter{ L'á', false });
                CHECK(letter != Letter{ L'Á', false });
            }
            {
                const Letter letter{ L'O', false };

                CHECK(letter == Letter{ L'o', false});
                CHECK(letter == Letter{ L'O', false});
                CHECK(letter != Letter{ L'p', false});
                CHECK(letter != Letter{ L'P', false});
                CHECK(letter != Letter{ L'0', false});
                CHECK(letter != Letter{ L'.', false});
                CHECK(letter != Letter{ L'ㅐ', false});
                CHECK(letter != Letter{ L'ㅒ', false});
                CHECK(letter != Letter{ L'ǒ', false});
                CHECK(letter != Letter{ L'ø', false });
            }
        }

        SUBCASE("라틴 문자 - 대소문자 구별 안 함 vs 대소문자 구별함")
        {
            {
                const Letter letter{ L'a', false };

                CHECK(letter != Letter{ L'a', true });
                CHECK(letter != Letter{ L'A', true });
                CHECK(letter != Letter{ L'b', true });
                CHECK(letter != Letter{ L'B', true });
                CHECK(letter != Letter{ L'1', true });
                CHECK(letter != Letter{ L'.', true });
                CHECK(letter != Letter{ L'ㅁ', true });
                CHECK(letter != Letter{ L'á', true });
                CHECK(letter != Letter{ L'Á', true });
            }
            {
                const Letter letter{ L'O', false };

                CHECK(letter != Letter{ L'o', true });
                CHECK(letter != Letter{ L'O', true });
                CHECK(letter != Letter{ L'p', true });
                CHECK(letter != Letter{ L'P', true });
                CHECK(letter != Letter{ L'0', true });
                CHECK(letter != Letter{ L'.', true });
                CHECK(letter != Letter{ L'ㅐ', true });
                CHECK(letter != Letter{ L'ㅒ', true });
                CHECK(letter != Letter{ L'ǒ', true });
                CHECK(letter != Letter{ L'ø', true });
            }
        }

        SUBCASE("라틴 문자 - 대소문자 구별함 vs 대소문자 구별 안 함")
        {
            {
                const Letter letter{ L'a', true };

                CHECK(letter != Letter{ L'a', false });
                CHECK(letter != Letter{ L'A', false });
                CHECK(letter != Letter{ L'b', false });
                CHECK(letter != Letter{ L'B', false });
                CHECK(letter != Letter{ L'1', false });
                CHECK(letter != Letter{ L'.', false });
                CHECK(letter != Letter{ L'ㅁ', false });
                CHECK(letter != Letter{ L'á', false });
                CHECK(letter != Letter{ L'Á', false });
            }
            {
                const Letter letter{ L'O', true };

                CHECK(letter != Letter{ L'o', false });
                CHECK(letter != Letter{ L'O', false });
                CHECK(letter != Letter{ L'p', false });
                CHECK(letter != Letter{ L'P', false });
                CHECK(letter != Letter{ L'0', false });
                CHECK(letter != Letter{ L'.', false });
                CHECK(letter != Letter{ L'ㅐ', false });
                CHECK(letter != Letter{ L'ㅒ', false });
                CHECK(letter != Letter{ L'ǒ', false });
                CHECK(letter != Letter{ L'ø', false });
            }
        }

        SUBCASE("라틴 문자 - 대소문자 구별함 vs 대소문자 구별함")
        {
            {
                const Letter letter{ L'a', true };

                CHECK(letter == Letter{ L'a', true });
                CHECK(letter != Letter{ L'A', true });
                CHECK(letter != Letter{ L'b', true });
                CHECK(letter != Letter{ L'B', true });
                CHECK(letter != Letter{ L'1', true });
                CHECK(letter != Letter{ L'.', true });
                CHECK(letter != Letter{ L'ㅁ', true });
                CHECK(letter != Letter{ L'á', true });
                CHECK(letter != Letter{ L'Á', true });
            }
            {
                const Letter letter{ L'O', true };

                CHECK(letter != Letter{ L'o', true });
                CHECK(letter == Letter{ L'O', true });
                CHECK(letter != Letter{ L'p', true });
                CHECK(letter != Letter{ L'P', true });
                CHECK(letter != Letter{ L'0', true });
                CHECK(letter != Letter{ L'.', true });
                CHECK(letter != Letter{ L'ㅐ', true });
                CHECK(letter != Letter{ L'ㅒ', true });
                CHECK(letter != Letter{ L'ǒ', true });
                CHECK(letter != Letter{ L'ø', true });
            }
        }

        SUBCASE("한글 문자")
        {
            {
                const Letter letter{ L'ㄱ' };

                CHECK(letter == Letter{ L'ㄱ' });
                CHECK(letter != Letter{ L'ᄀ' });  // 초성(U+1100)
                CHECK(letter != Letter{ L'ᆨ' });  // 종성(U+11A8)
                CHECK(letter != Letter{ L'ㄲ' });
                CHECK(letter != Letter{ L'ㄴ' });
                CHECK(letter != Letter{ L'r', false });
                CHECK(letter != Letter{ L'R', false });
                CHECK(letter != Letter{ L'r', true });
                CHECK(letter != Letter{ L'R', true });
                CHECK(letter != Letter{ L'1' });
                CHECK(letter != Letter{ L'.' });
            }
            {
                const Letter letter{ L'가' };

                CHECK(letter == Letter{ L'가' });
                CHECK(letter != Letter{ L'각' });
                CHECK(letter != Letter{ L'나' });
                CHECK(letter != Letter{ L'r', false });
                CHECK(letter != Letter{ L'R', false });
                CHECK(letter != Letter{ L'r', true });
                CHECK(letter != Letter{ L'R', true });
                CHECK(letter != Letter{ L'1' });
                CHECK(letter != Letter{ L'.' });
            }
            {
                const Letter letter{ L'쒡' };

                CHECK(letter == Letter{ L'쒡' });
                CHECK(letter != Letter{ L'쒠' });
                CHECK(letter != Letter{ L'쒜' });
                CHECK(letter != Letter{ L'쑤' });
                CHECK(letter != Letter{ L'ㅆ' });
                CHECK(letter != Letter{ L'쒢' });
                CHECK(letter != Letter{ L't', false });
                CHECK(letter != Letter{ L'T', false });
                CHECK(letter != Letter{ L't', true });
                CHECK(letter != Letter{ L'T', true });
                CHECK(letter != Letter{ L'1' });
                CHECK(letter != Letter{ L'.' });
            }
        }

        SUBCASE("doNeedFullComposite은 아무 영향 없음")
        {
            {
                const Letter letter{ L'a', false };

                CHECK(letter != Letter{ L'ㅁ', false, true });
            }
            {
                const Letter letter{ L'O', false };

                CHECK(letter != Letter{ L'ㅐ', false, true });
                CHECK(letter != Letter{ L'ㅒ', false, true });
            }

            {
                const Letter letter{ L'a', false };

                CHECK(letter != Letter{ L'ㅁ', true, true });
            }
            {
                const Letter letter{ L'O', false };

                CHECK(letter != Letter{ L'ㅐ', true, true });
                CHECK(letter != Letter{ L'ㅒ', true, true });
            }

            {
                const Letter letter{ L'a', true };

                CHECK(letter != Letter{ L'ㅁ', false, true });
            }
            {
                const Letter letter{ L'O', true };

                CHECK(letter != Letter{ L'ㅐ', false, true });
                CHECK(letter != Letter{ L'ㅒ', false, true });
            }

            {
                const Letter letter{ L'a', true };

                CHECK(letter != Letter{ L'ㅁ', true, true });
            }
            {
                const Letter letter{ L'O', true };

                CHECK(letter != Letter{ L'ㅐ', true, true });
                CHECK(letter != Letter{ L'ㅒ', true, true });
            }

            {
                const Letter letter{ L'ㄱ', false, true };

                CHECK(letter == Letter{ L'ㄱ' });
            }
            {
                const Letter letter{ L'ㄱ' };

                CHECK(letter == Letter{ L'ㄱ', false, true });
            }
            {
                const Letter letter{ L'ㄱ', false, true };

                CHECK(letter == Letter{ L'ㄱ', false, true });
            }
            {
                const Letter letter{ L'가', false, true };

                CHECK(letter == Letter{ L'가' });
            }
            {
                const Letter letter{ L'가' };

                CHECK(letter == Letter{ L'가', false, true });
            }
            {
                const Letter letter{ L'가', false, true };

                CHECK(letter == Letter{ L'가', false, true });
            }
            {
                const Letter letter{ L'쒡', false, true };

                CHECK(letter == Letter{ L'쒡' });
                CHECK(letter != Letter{ L'쒠' });
                CHECK(letter != Letter{ L'쒜' });
                CHECK(letter != Letter{ L'쑤' });
                CHECK(letter != Letter{ L'ㅆ' });
                CHECK(letter != Letter{ L'쒢' });
            }
            {
                const Letter letter{ L'쒡' };

                CHECK(letter == Letter{ L'쒡', false, true });
                CHECK(letter != Letter{ L'쒠', false, true });
                CHECK(letter != Letter{ L'쒜', false, true });
                CHECK(letter != Letter{ L'쑤', false, true });
                CHECK(letter != Letter{ L'ㅆ', false, true });
                CHECK(letter != Letter{ L'쒢', false, true });
            }
            {
                const Letter letter{ L'쒡', false, true };

                CHECK(letter == Letter{ L'쒡', false, true });
                CHECK(letter != Letter{ L'쒠', false, true });
                CHECK(letter != Letter{ L'쒜', false, true });
                CHECK(letter != Letter{ L'쑤', false, true });
                CHECK(letter != Letter{ L'ㅆ', false, true });
                CHECK(letter != Letter{ L'쒢', false, true });
            }
        }
    }

    TEST_CASE("operator<=>(const Letter& other)")
    {
        SUBCASE("라틴 문자 - 대소문자 구별 안 함")
        {
            const Letter letter{ L'a', false };

            CHECK((letter <=> Letter{ L'a', false }) == std::strong_ordering::equal);
            CHECK((letter <=> Letter{ L'A', false }) == (L'a' <=> L'A'));
            CHECK((letter <=> Letter{ L'a', true }) == std::strong_ordering::greater);
            CHECK((letter <=> Letter{ L'A', true }) == std::strong_ordering::greater);
            CHECK((letter <=> Letter{ L'b', false }) == std::strong_ordering::less);
            CHECK((letter <=> Letter{ L'B', false }) == std::strong_ordering::less);
            CHECK((letter <=> Letter{ L'b', true }) == std::strong_ordering::less);
            CHECK((letter <=> Letter{ L'B', true }) == std::strong_ordering::less);
            CHECK((letter <=> Letter{ L'1' }) == (L'a' <=> L'1'));
            CHECK((letter <=> Letter{ L'.' }) == (L'a' <=> L'.'));
            CHECK((letter <=> Letter{ L'ㅁ' }) == (L'a' <=> L'ㅁ'));
            CHECK((letter <=> Letter{ L'á', false }) == (L'a' <=> L'á'));
            CHECK((letter <=> Letter{ L'Á', false }) == (L'a' <=> L'Á'));
            CHECK((letter <=> Letter{ L'á', true }) == (L'a' <=> L'á'));
            CHECK((letter <=> Letter{ L'Á', true }) == (L'a' <=> L'Á'));
        }

        SUBCASE("라틴 문자 - 대소문자 구별함")
        {
            const Letter letter{ L'a', true };

            CHECK((letter <=> Letter{ L'a', false }) == std::strong_ordering::less);
            CHECK((letter <=> Letter{ L'A', false }) == std::strong_ordering::less);
            CHECK((letter <=> Letter{ L'a', true }) == std::strong_ordering::equal);
            CHECK((letter <=> Letter{ L'A', true }) == (L'a' <=> L'A'));
            CHECK((letter <=> Letter{ L'b', false }) == std::strong_ordering::less);
            CHECK((letter <=> Letter{ L'B', false }) == std::strong_ordering::less);
            CHECK((letter <=> Letter{ L'b', true }) == std::strong_ordering::less);
            CHECK((letter <=> Letter{ L'B', true }) == std::strong_ordering::less);
            CHECK((letter <=> Letter{ L'1' }) == (L'a' <=> L'1'));
            CHECK((letter <=> Letter{ L'.' }) == (L'a' <=> L'.'));
            CHECK((letter <=> Letter{ L'ㅁ' }) == (L'a' <=> L'ㅁ'));
            CHECK((letter <=> Letter{ L'á', false }) == (L'a' <=> L'á'));
            CHECK((letter <=> Letter{ L'Á', false }) == (L'a' <=> L'Á'));
            CHECK((letter <=> Letter{ L'á', true }) == (L'a' <=> L'á'));
            CHECK((letter <=> Letter{ L'Á', true }) == (L'a' <=> L'Á'));
        }

        SUBCASE("한글 문자")
        {
            const Letter letter{ L'쒡' };

            CHECK((letter <=> Letter{ L'쒡' }) == std::strong_ordering::equal);
            CHECK((letter <=> Letter{ L'쒠' }) == (L'쒡' <=> L'쒠'));
            CHECK((letter <=> Letter{ L'쒜' }) == (L'쒡' <=> L'쒜'));
            CHECK((letter <=> Letter{ L'쑤' }) == (L'쒡' <=> L'쑤'));
            CHECK((letter <=> Letter{ L'ㅆ' }) == (L'쒡' <=> L'ㅆ'));
            CHECK((letter <=> Letter{ L'쒢' }) == (L'쒡' <=> L'쒢'));
            CHECK((letter <=> Letter{ L't' }) == (L'쒡' <=> L't'));
            CHECK((letter <=> Letter{ L'T' }) == (L'쒡' <=> L'T'));
            CHECK((letter <=> Letter{ L'1' }) == (L'쒡' <=> L'1'));
            CHECK((letter <=> Letter{ L'.' }) == (L'쒡' <=> L'.'));
        }

        SUBCASE("doNeedFullComposite은 아무 영향 없음")
        {
            {
                const Letter letter{ L'쒡', false, true };

                CHECK((letter <=> Letter{ L'쒡' }) == std::strong_ordering::equal);
                CHECK((letter <=> Letter{ L'쒠' }) == (L'쒡' <=> L'쒠'));
                CHECK((letter <=> Letter{ L'쒜' }) == (L'쒡' <=> L'쒜'));
                CHECK((letter <=> Letter{ L'쑤' }) == (L'쒡' <=> L'쑤'));
                CHECK((letter <=> Letter{ L'ㅆ' }) == (L'쒡' <=> L'ㅆ'));
                CHECK((letter <=> Letter{ L'쒢' }) == (L'쒡' <=> L'쒢'));
                CHECK((letter <=> Letter{ L't' }) == (L'쒡' <=> L't'));
                CHECK((letter <=> Letter{ L'T' }) == (L'쒡' <=> L'T'));
                CHECK((letter <=> Letter{ L'1' }) == (L'쒡' <=> L'1'));
                CHECK((letter <=> Letter{ L'.' }) == (L'쒡' <=> L'.'));
            }
            {
                const Letter letter{ L'쒡' };

                CHECK((letter <=> Letter{ L'쒡', false, true }) == std::strong_ordering::equal);
                CHECK((letter <=> Letter{ L'쒠', false, true }) == (L'쒡' <=> L'쒠'));
                CHECK((letter <=> Letter{ L'쒜', false, true }) == (L'쒡' <=> L'쒜'));
                CHECK((letter <=> Letter{ L'쑤', false, true }) == (L'쒡' <=> L'쑤'));
                CHECK((letter <=> Letter{ L'ㅆ', false, true }) == (L'쒡' <=> L'ㅆ'));
                CHECK((letter <=> Letter{ L'쒢', false, true }) == (L'쒡' <=> L'쒢'));
                CHECK((letter <=> Letter{ L't', false, true }) == (L'쒡' <=> L't'));
                CHECK((letter <=> Letter{ L'T', false, true }) == (L'쒡' <=> L'T'));
                CHECK((letter <=> Letter{ L'1', false, true }) == (L'쒡' <=> L'1'));
                CHECK((letter <=> Letter{ L'.', false, true }) == (L'쒡' <=> L'.'));
            }
            {
                const Letter letter{ L'쒡', false, true };

                CHECK((letter <=> Letter{ L'쒡', false, true }) == std::strong_ordering::equal);
                CHECK((letter <=> Letter{ L'쒠', false, true }) == (L'쒡' <=> L'쒠'));
                CHECK((letter <=> Letter{ L'쒜', false, true }) == (L'쒡' <=> L'쒜'));
                CHECK((letter <=> Letter{ L'쑤', false, true }) == (L'쒡' <=> L'쑤'));
                CHECK((letter <=> Letter{ L'ㅆ', false, true }) == (L'쒡' <=> L'ㅆ'));
                CHECK((letter <=> Letter{ L'쒢', false, true }) == (L'쒡' <=> L'쒢'));
                CHECK((letter <=> Letter{ L't', false, true }) == (L'쒡' <=> L't'));
                CHECK((letter <=> Letter{ L'T', false, true }) == (L'쒡' <=> L'T'));
                CHECK((letter <=> Letter{ L'1', false, true }) == (L'쒡' <=> L'1'));
                CHECK((letter <=> Letter{ L'.', false, true }) == (L'쒡' <=> L'.'));
            }
        }

        SUBCASE("정렬")
        {
            constexpr int times = 1'000;

            Letter letters[] = {
                Letter{ L'a', false }, Letter{ L'A', false },
                Letter{ L'a', true },  Letter{ L'A', true },
                Letter{ L'b', false }, Letter{ L'B', false },
                Letter{ L'b', true },  Letter{ L'B', true },
                Letter{ L'1' }, Letter{ L'.' }, Letter{ L' ' },
                Letter{ L'ㄱ' }, Letter{ L'나' }, Letter{ L'ㅢ' }, Letter{ L'쒡' }
            };

            const Letter expected[] = {
                Letter{ L' ' },
                Letter{ L'.' },
                Letter{ L'1' },
                Letter{ L'A', true },
                Letter{ L'a', true },
                Letter{ L'A', false },
                Letter{ L'a', false },
                Letter{ L'B', true },
                Letter{ L'b', true },
                Letter{ L'B', false },
                Letter{ L'b', false },
                Letter{ L'ㄱ' },
                Letter{ L'ㅢ' },
                Letter{ L'나' },
                Letter{ L'쒡' }
            };

            std::random_device rd{};
            std::mt19937 generator{ rd() };

            for (int i = 0; i < times; i++)
            {
                std::ranges::shuffle(letters, generator);
                std::ranges::sort(letters);
                CHECK(std::ranges::equal(letters, expected));
            }
        }
    }
}
