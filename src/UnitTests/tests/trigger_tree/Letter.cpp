#include <doctest.h>

#include <algorithm>
#include <random>
#include <span>

#include "Typoon/trigger_tree/Letter.h"

#include "util/DocTestWStringSupport.h"


using typoon::core::Letter;

TEST_SUITE("Letter")
{
    TEST_CASE("operator<(const Letter& other)")
    {
        SUBCASE("라틴 문자")
        {
            CHECK((Letter{ L'a', false } < Letter{ L'a', true  }) == true );
            CHECK((Letter{ L'a', true  } < Letter{ L'a', false }) == false);

            CHECK((Letter{ L'a', false } < Letter{ L'A', false }) == false);
            CHECK((Letter{ L'A', false } < Letter{ L'a', false }) == false);

            CHECK((Letter{ L'a', false } < Letter{ L'A', true  }) == false);
            CHECK((Letter{ L'A', true  } < Letter{ L'a', false }) == true );

            CHECK((Letter{ L'a', true  } < Letter{ L'A', false }) == false);
            CHECK((Letter{ L'A', false } < Letter{ L'a', true  }) == true );

            CHECK((Letter{ L'a', true  } < Letter{ L'A', true  }) == false);
            CHECK((Letter{ L'A', true  } < Letter{ L'a', true  }) == true );
            
            CHECK((Letter{ L'A', false } < Letter{ L'A', true  }) == false);
            CHECK((Letter{ L'A', true  } < Letter{ L'A', false }) == true );

            CHECK((Letter{ L'a', false } < Letter{ L'a', false }) == false);
            CHECK((Letter{ L'a', true  } < Letter{ L'a', true  }) == false);
            CHECK((Letter{ L'A', false } < Letter{ L'A', false }) == false);
            CHECK((Letter{ L'A', true  } < Letter{ L'A', true  }) == false);


            CHECK((Letter{ L'a', false } < Letter{ L'b', false }) == (L'a' < L'b'));
            CHECK((Letter{ L'a', false } < Letter{ L'B', false }) == (L'a' < L'b'));
            CHECK((Letter{ L'a', false } < Letter{ L'b', true  }) == (L'a' < L'b'));
            CHECK((Letter{ L'a', false } < Letter{ L'B', true  }) == (L'a' < L'b'));
            CHECK((Letter{ L'a', true  } < Letter{ L'b', false }) == (L'a' < L'b'));
            CHECK((Letter{ L'a', true  } < Letter{ L'B', false }) == (L'a' < L'b'));
            CHECK((Letter{ L'a', true  } < Letter{ L'b', true  }) == (L'a' < L'b'));
            CHECK((Letter{ L'a', true  } < Letter{ L'B', true  }) == (L'a' < L'b'));

            CHECK((Letter{ L'b', false } < Letter{ L'a', false }) == (L'b' < L'a'));
            CHECK((Letter{ L'B', false } < Letter{ L'a', false }) == (L'b' < L'a'));
            CHECK((Letter{ L'b', true  } < Letter{ L'a', false }) == (L'b' < L'a'));
            CHECK((Letter{ L'B', true  } < Letter{ L'a', false }) == (L'b' < L'a'));
            CHECK((Letter{ L'b', false } < Letter{ L'a', true  }) == (L'b' < L'a'));
            CHECK((Letter{ L'B', false } < Letter{ L'a', true  }) == (L'b' < L'a'));
            CHECK((Letter{ L'b', true  } < Letter{ L'a', true  }) == (L'b' < L'a'));
            CHECK((Letter{ L'B', true  } < Letter{ L'a', true  }) == (L'b' < L'a'));


            CHECK((Letter{ L'a', false } < Letter{ L'1' }) == (L'a' < L'1'));
            CHECK((Letter{ L'a', true  } < Letter{ L'1' }) == (L'a' < L'1'));
            CHECK((Letter{ L'A', false } < Letter{ L'1' }) == (L'a' < L'1'));
            CHECK((Letter{ L'A', true  } < Letter{ L'1' }) == (L'a' < L'1'));

            CHECK((Letter{ L'1' } < Letter{ L'a', false }) == (L'1' < L'a'));
            CHECK((Letter{ L'1' } < Letter{ L'a', true  }) == (L'1' < L'a'));
            CHECK((Letter{ L'1' } < Letter{ L'A', false }) == (L'1' < L'a'));
            CHECK((Letter{ L'1' } < Letter{ L'A', true  }) == (L'1' < L'a'));


            CHECK((Letter{ L'a', false } < Letter{ L'.' }) == (L'a' < L'.'));
            CHECK((Letter{ L'a', true  } < Letter{ L'.' }) == (L'a' < L'.'));
            CHECK((Letter{ L'A', false } < Letter{ L'.' }) == (L'a' < L'.'));
            CHECK((Letter{ L'A', true  } < Letter{ L'.' }) == (L'a' < L'.'));

            CHECK((Letter{ L'.' } < Letter{ L'a', false }) == (L'.' < L'a'));
            CHECK((Letter{ L'.' } < Letter{ L'a', true  }) == (L'.' < L'a'));
            CHECK((Letter{ L'.' } < Letter{ L'A', false }) == (L'.' < L'a'));
            CHECK((Letter{ L'.' } < Letter{ L'A', true  }) == (L'.' < L'a'));


            CHECK((Letter{ L'a', false } < Letter{ L'ㅁ' }) == (L'a' < L'ㅁ'));
            CHECK((Letter{ L'a', true  } < Letter{ L'ㅁ' }) == (L'a' < L'ㅁ'));
            CHECK((Letter{ L'A', false } < Letter{ L'ㅁ' }) == (L'a' < L'ㅁ'));
            CHECK((Letter{ L'A', true  } < Letter{ L'ㅁ' }) == (L'a' < L'ㅁ'));

            CHECK((Letter{ L'ㅁ' } < Letter{ L'a', false }) == (L'ㅁ' < L'a'));
            CHECK((Letter{ L'ㅁ' } < Letter{ L'a', true  }) == (L'ㅁ' < L'a'));
            CHECK((Letter{ L'ㅁ' } < Letter{ L'A', false }) == (L'ㅁ' < L'a'));
            CHECK((Letter{ L'ㅁ' } < Letter{ L'A', true  }) == (L'ㅁ' < L'a'));


            CHECK((Letter{ L'a', false } < Letter{ L'ㅁ' }) == (L'a' < L'ㅁ'));
            CHECK((Letter{ L'a', true  } < Letter{ L'ㅁ' }) == (L'a' < L'ㅁ'));
            CHECK((Letter{ L'A', false } < Letter{ L'ㅁ' }) == (L'a' < L'ㅁ'));
            CHECK((Letter{ L'A', true  } < Letter{ L'ㅁ' }) == (L'a' < L'ㅁ'));

            CHECK((Letter{ L'ㅁ' } < Letter{ L'a', false }) == (L'ㅁ' < L'a'));
            CHECK((Letter{ L'ㅁ' } < Letter{ L'a', true  }) == (L'ㅁ' < L'a'));
            CHECK((Letter{ L'ㅁ' } < Letter{ L'A', false }) == (L'ㅁ' < L'a'));
            CHECK((Letter{ L'ㅁ' } < Letter{ L'A', true  }) == (L'ㅁ' < L'a'));


            CHECK((Letter{ L'a', false } < Letter{ L'á', false }) == (L'a' < L'á'));
            CHECK((Letter{ L'a', false } < Letter{ L'Á', false }) == (L'a' < L'Á'));
            CHECK((Letter{ L'a', false } < Letter{ L'á', true  }) == (L'a' < L'á'));
            CHECK((Letter{ L'a', false } < Letter{ L'Á', true  }) == (L'a' < L'Á'));
            CHECK((Letter{ L'a', true  } < Letter{ L'á', false }) == (L'a' < L'á'));
            CHECK((Letter{ L'a', true  } < Letter{ L'Á', false }) == (L'a' < L'Á'));
            CHECK((Letter{ L'a', true  } < Letter{ L'á', true  }) == (L'a' < L'á'));
            CHECK((Letter{ L'a', true  } < Letter{ L'Á', true  }) == (L'a' < L'Á'));

            CHECK((Letter{ L'á', false } < Letter{ L'a', false }) == (L'á' < L'a'));
            CHECK((Letter{ L'Á', false } < Letter{ L'a', false }) == (L'Á' < L'a'));
            CHECK((Letter{ L'á', true  } < Letter{ L'a', false }) == (L'á' < L'a'));
            CHECK((Letter{ L'Á', true  } < Letter{ L'a', false }) == (L'Á' < L'a'));
            CHECK((Letter{ L'á', false } < Letter{ L'a', true  }) == (L'á' < L'a'));
            CHECK((Letter{ L'Á', false } < Letter{ L'a', true  }) == (L'Á' < L'a'));
            CHECK((Letter{ L'á', true  } < Letter{ L'a', true  }) == (L'á' < L'a'));
            CHECK((Letter{ L'Á', true  } < Letter{ L'a', true  }) == (L'Á' < L'a'));
        }

        SUBCASE("한글 문자")
        {
            const Letter letter{ L'쒡' };

            CHECK((letter < Letter{ L'쒡' }) == (L'쒡' < L'쒡'));
            CHECK((letter < Letter{ L'쒠' }) == (L'쒡' < L'쒠'));
            CHECK((letter < Letter{ L'쒜' }) == (L'쒡' < L'쒜'));
            CHECK((letter < Letter{ L'쑤' }) == (L'쒡' < L'쑤'));
            CHECK((letter < Letter{ L'ㅆ' }) == (L'쒡' < L'ㅆ'));
            CHECK((letter < Letter{ L'쒢' }) == (L'쒡' < L'쒢'));
            CHECK((letter < Letter{ L't' }) == (L'쒡' < L't'));
            CHECK((letter < Letter{ L'T' }) == (L'쒡' < L'T'));
            CHECK((letter < Letter{ L'1' }) == (L'쒡' < L'1'));
            CHECK((letter < Letter{ L'.' }) == (L'쒡' < L'.'));
        }

        SUBCASE("doNeedFullComposite은 아무 영향 없음")
        {
            {
                const Letter letter{ L'쒡', false, true };

                CHECK((letter < Letter{ L'쒡' }) == (L'쒡' < L'쒡'));
                CHECK((letter < Letter{ L'쒠' }) == (L'쒡' < L'쒠'));
                CHECK((letter < Letter{ L'쒜' }) == (L'쒡' < L'쒜'));
                CHECK((letter < Letter{ L'쑤' }) == (L'쒡' < L'쑤'));
                CHECK((letter < Letter{ L'ㅆ' }) == (L'쒡' < L'ㅆ'));
                CHECK((letter < Letter{ L'쒢' }) == (L'쒡' < L'쒢'));
                CHECK((letter < Letter{ L't' }) == (L'쒡' < L't'));
                CHECK((letter < Letter{ L'T' }) == (L'쒡' < L'T'));
                CHECK((letter < Letter{ L'1' }) == (L'쒡' < L'1'));
                CHECK((letter < Letter{ L'.' }) == (L'쒡' < L'.'));
            }
            {
                const Letter letter{ L'쒡' };

                CHECK((letter < Letter{ L'쒡', false, true }) == (L'쒡' < L'쒡'));
                CHECK((letter < Letter{ L'쒠', false, true }) == (L'쒡' < L'쒠'));
                CHECK((letter < Letter{ L'쒜', false, true }) == (L'쒡' < L'쒜'));
                CHECK((letter < Letter{ L'쑤', false, true }) == (L'쒡' < L'쑤'));
                CHECK((letter < Letter{ L'ㅆ', false, true }) == (L'쒡' < L'ㅆ'));
                CHECK((letter < Letter{ L'쒢', false, true }) == (L'쒡' < L'쒢'));
                CHECK((letter < Letter{ L't', false, true }) == (L'쒡' < L't'));
                CHECK((letter < Letter{ L'T', false, true }) == (L'쒡' < L'T'));
                CHECK((letter < Letter{ L'1', false, true }) == (L'쒡' < L'1'));
                CHECK((letter < Letter{ L'.', false, true }) == (L'쒡' < L'.'));
            }
            {
                const Letter letter{ L'쒡', false, true };

                CHECK((letter < Letter{ L'쒡', false, true }) == (L'쒡' < L'쒡'));
                CHECK((letter < Letter{ L'쒠', false, true }) == (L'쒡' < L'쒠'));
                CHECK((letter < Letter{ L'쒜', false, true }) == (L'쒡' < L'쒜'));
                CHECK((letter < Letter{ L'쑤', false, true }) == (L'쒡' < L'쑤'));
                CHECK((letter < Letter{ L'ㅆ', false, true }) == (L'쒡' < L'ㅆ'));
                CHECK((letter < Letter{ L'쒢', false, true }) == (L'쒡' < L'쒢'));
                CHECK((letter < Letter{ L't', false, true }) == (L'쒡' < L't'));
                CHECK((letter < Letter{ L'T', false, true }) == (L'쒡' < L'T'));
                CHECK((letter < Letter{ L'1', false, true }) == (L'쒡' < L'1'));
                CHECK((letter < Letter{ L'.', false, true }) == (L'쒡' < L'.'));
            }
        }

        SUBCASE("특수 문자")
        {
            CHECK((Letter{ Letter::NON_WORD_LETTER } < Letter{ L'a', false }) == true );
            CHECK((Letter{ L'a', false } < Letter{ Letter::NON_WORD_LETTER }) == false);

            CHECK((Letter{ Letter::NON_WORD_LETTER } < Letter{ L'a', true  }) == true );
            CHECK((Letter{ L'a', true  } < Letter{ Letter::NON_WORD_LETTER }) == false);

            CHECK((Letter{ Letter::NON_WORD_LETTER } < Letter{ L'A', false }) == true );
            CHECK((Letter{ L'A', false } < Letter{ Letter::NON_WORD_LETTER }) == false);

            CHECK((Letter{ Letter::NON_WORD_LETTER } < Letter{ L'A', true  }) == true );
            CHECK((Letter{ L'A', true  } < Letter{ Letter::NON_WORD_LETTER }) == false);


            CHECK((Letter{ Letter::NON_WORD_LETTER } < Letter{ L'1' }) == true );
            CHECK((Letter{ L'1' } < Letter{ Letter::NON_WORD_LETTER }) == false);


            CHECK((Letter{ Letter::NON_WORD_LETTER } < Letter{ L'.' }) == true );
            CHECK((Letter{ L'.' } < Letter{ Letter::NON_WORD_LETTER }) == false);


            CHECK((Letter{ Letter::NON_WORD_LETTER } < Letter{ L'ㅁ' }) == true );
            CHECK((Letter{ L'ㅁ' } < Letter{ Letter::NON_WORD_LETTER }) == false);


            CHECK((Letter{ Letter::NON_WORD_LETTER } < Letter{ Letter::LAST_INPUT_LETTER }) == (Letter::NON_WORD_LETTER < Letter::LAST_INPUT_LETTER));
            CHECK((Letter{ Letter::LAST_INPUT_LETTER } < Letter{ Letter::NON_WORD_LETTER }) == (Letter::LAST_INPUT_LETTER < Letter::NON_WORD_LETTER));


            CHECK((Letter{ Letter::NON_WORD_LETTER } < Letter{ Letter::NON_WORD_LETTER }) == false);
            CHECK((Letter{ Letter::LAST_INPUT_LETTER } < Letter{ Letter::LAST_INPUT_LETTER }) == false);
        }

        SUBCASE("정렬")
        {
            constexpr int times = 1'000;

            Letter letters[] = {
                Letter{ L'a', false },
                Letter{ L'a', true },  Letter{ L'A', true },

                Letter{ L'b', false },
                Letter{ L'b', true },  Letter{ L'B', true },

                Letter{ L'1' }, Letter{ L'.' }, Letter{ L' ' },
                Letter{ L'{' }, Letter{ L'|' }, Letter{ L'~' },
                Letter{ L'ㄱ' }, Letter{ L'나' }, Letter{ L'ㅢ' }, Letter{ L'쒡' },

                Letter{ Letter::NON_WORD_LETTER },
            };

            const Letter expected[] = {
                Letter{ Letter::NON_WORD_LETTER },

                Letter{ L' ' },
                Letter{ L'.' },
                Letter{ L'1' },

                Letter{ L'A', true  },
                Letter{ L'a', false },
                Letter{ L'a', true  },

                Letter{ L'B', true  },
                Letter{ L'b', false },
                Letter{ L'b', true  },

                Letter{ L'{' },
                Letter{ L'|' },
                Letter{ L'~' },

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
                std::sort(std::begin(letters), std::end(letters));
                const bool isSorted = std::ranges::equal(letters, expected);
                CHECK(isSorted);
            }
        }
    }

    TEST_CASE("operator<(wchar_t ch)")
    {
        SUBCASE("라틴 문자")
        {
            CHECK((Letter{ L'a', false } < L'a') == false);
            CHECK((L'a' < Letter{ L'a', false }) == false);

            CHECK((Letter{ L'a', true  } < L'a') == false);
            CHECK((L'a' < Letter{ L'a', true  }) == false);

            CHECK((Letter{ L'a', false } < L'A') == false);
            CHECK((L'A' < Letter{ L'a', false }) == false);

            CHECK((Letter{ L'a', true  } < L'A') == false);
            CHECK((L'A' < Letter{ L'a', true  }) == true );

            
            CHECK((Letter{ L'A', false } < L'a') == false);
            CHECK((L'a' < Letter{ L'A', false }) == false);

            CHECK((Letter{ L'A', true  } < L'a') == true );
            CHECK((L'a' < Letter{ L'A', true  }) == false);

            CHECK((Letter{ L'A', false } < L'A') == false);
            CHECK((L'A' < Letter{ L'A', false }) == false);

            CHECK((Letter{ L'A', true  } < L'A') == false);
            CHECK((L'A' < Letter{ L'A', true  }) == false);

            
            CHECK((Letter{ L'a', false } < L'b') == (L'a' < L'b'));
            CHECK((L'b' < Letter{ L'a', false }) == (L'b' < L'a'));

            CHECK((Letter{ L'a', true  } < L'b') == (L'a' < L'b'));
            CHECK((L'b' < Letter{ L'a', true  }) == (L'b' < L'a'));

            CHECK((Letter{ L'a', false } < L'B') == (L'a' < L'b'));
            CHECK((L'B' < Letter{ L'a', false }) == (L'b' < L'a'));

            CHECK((Letter{ L'a', true  } < L'B') == (L'a' < L'b'));
            CHECK((L'B' < Letter{ L'a', true  }) == (L'b' < L'a'));

            
            CHECK((Letter{ L'A', false } < L'b') == (L'a' < L'b'));
            CHECK((L'b' < Letter{ L'A', false }) == (L'b' < L'a'));

            CHECK((Letter{ L'A', true  } < L'b') == (L'a' < L'b'));
            CHECK((L'b' < Letter{ L'A', true  }) == (L'b' < L'a'));

            CHECK((Letter{ L'A', false } < L'B') == (L'a' < L'b'));
            CHECK((L'B' < Letter{ L'A', false }) == (L'b' < L'a'));

            CHECK((Letter{ L'A', true  } < L'B') == (L'a' < L'b'));
            CHECK((L'B' < Letter{ L'A', true  }) == (L'b' < L'a'));

            CHECK((Letter{ L'a', false } < L'1') == (L'a' < L'1'));
            CHECK((Letter{ L'a', true  } < L'1') == (L'a' < L'1'));
            CHECK((Letter{ L'A', false } < L'1') == (L'a' < L'1'));
            CHECK((Letter{ L'A', true  } < L'1') == (L'a' < L'1'));

            CHECK((L'1' < Letter{ L'a', false }) == (L'1' < L'a'));
            CHECK((L'1' < Letter{ L'a', true  }) == (L'1' < L'a'));
            CHECK((L'1' < Letter{ L'A', false }) == (L'1' < L'a'));
            CHECK((L'1' < Letter{ L'A', true  }) == (L'1' < L'a'));


            CHECK((Letter{ L'a', false } < L'.') == (L'a' < L'.'));
            CHECK((Letter{ L'a', true  } < L'.') == (L'a' < L'.'));
            CHECK((Letter{ L'A', false } < L'.') == (L'a' < L'.'));
            CHECK((Letter{ L'A', true  } < L'.') == (L'a' < L'.'));

            CHECK((L'.' < Letter{ L'a', false }) == (L'.' < L'a'));
            CHECK((L'.' < Letter{ L'a', true  }) == (L'.' < L'a'));
            CHECK((L'.' < Letter{ L'A', false }) == (L'.' < L'a'));
            CHECK((L'.' < Letter{ L'A', true  }) == (L'.' < L'a'));


            CHECK((Letter{ L'a', false } < L'ㅁ') == (L'a' < L'ㅁ'));
            CHECK((Letter{ L'a', true  } < L'ㅁ') == (L'a' < L'ㅁ'));
            CHECK((Letter{ L'A', false } < L'ㅁ') == (L'a' < L'ㅁ'));
            CHECK((Letter{ L'A', true  } < L'ㅁ') == (L'a' < L'ㅁ'));

            CHECK((L'ㅁ' < Letter{ L'a', false }) == (L'ㅁ' < L'a'));
            CHECK((L'ㅁ' < Letter{ L'a', true  }) == (L'ㅁ' < L'a'));
            CHECK((L'ㅁ' < Letter{ L'A', false }) == (L'ㅁ' < L'a'));
            CHECK((L'ㅁ' < Letter{ L'A', true  }) == (L'ㅁ' < L'a'));


            CHECK((Letter{ L'a', false } < L'á') == (L'a' < L'á'));
            CHECK((Letter{ L'a', false } < L'Á') == (L'a' < L'Á'));
            CHECK((Letter{ L'a', true  } < L'á') == (L'a' < L'á'));
            CHECK((Letter{ L'a', true  } < L'Á') == (L'a' < L'Á'));

            CHECK((L'á' < Letter{ L'a', false }) == (L'á' < L'a'));
            CHECK((L'Á' < Letter{ L'a', false }) == (L'Á' < L'a'));
            CHECK((L'á' < Letter{ L'a', true  }) == (L'á' < L'a'));
            CHECK((L'Á' < Letter{ L'a', true  }) == (L'Á' < L'a'));
        }

        SUBCASE("한글 문자")
        {
            const Letter letter{ L'쒡' };

            CHECK((letter < L'쒡') == (L'쒡' < L'쒡'));
            CHECK((letter < L'쒠') == (L'쒡' < L'쒠'));
            CHECK((letter < L'쒜') == (L'쒡' < L'쒜'));
            CHECK((letter < L'쑤') == (L'쒡' < L'쑤'));
            CHECK((letter < L'ㅆ') == (L'쒡' < L'ㅆ'));
            CHECK((letter < L'쒢') == (L'쒡' < L'쒢'));
            CHECK((letter < L't') == (L'쒡' < L't'));
            CHECK((letter < L'T') == (L'쒡' < L'T'));
            CHECK((letter < L'1') == (L'쒡' < L'1'));
            CHECK((letter < L'.') == (L'쒡' < L'.'));

            CHECK((L'쒡' < letter) == (L'쒡' < L'쒡'));
            CHECK((L'쒠' < letter) == (L'쒠' < L'쒡'));
            CHECK((L'쒜' < letter) == (L'쒜' < L'쒡'));
            CHECK((L'쑤' < letter) == (L'쑤' < L'쒡'));
            CHECK((L'ㅆ' < letter) == (L'ㅆ' < L'쒡'));
            CHECK((L'쒢' < letter) == (L'쒢' < L'쒡'));
            CHECK((L't' < letter) == (L't' < L'쒡'));
            CHECK((L'T' < letter) == (L'T' < L'쒡'));
            CHECK((L'1' < letter) == (L'1' < L'쒡'));
            CHECK((L'.' < letter) == (L'.' < L'쒡'));
        }

        SUBCASE("doNeedFullComposite은 아무 영향 없음")
        {
            {
                const Letter letter{ L'쒡', false, true };

                CHECK((letter < L'쒡') == (L'쒡' < L'쒡'));
                CHECK((letter < L'쒠') == (L'쒡' < L'쒠'));
                CHECK((letter < L'쒜') == (L'쒡' < L'쒜'));
                CHECK((letter < L'쑤') == (L'쒡' < L'쑤'));
                CHECK((letter < L'ㅆ') == (L'쒡' < L'ㅆ'));
                CHECK((letter < L'쒢') == (L'쒡' < L'쒢'));
                CHECK((letter < L't') == (L'쒡' < L't'));
                CHECK((letter < L'T') == (L'쒡' < L'T'));
                CHECK((letter < L'1') == (L'쒡' < L'1'));
                CHECK((letter < L'.') == (L'쒡' < L'.'));

                CHECK((L'쒡' < letter) == (L'쒡' < L'쒡'));
                CHECK((L'쒠' < letter) == (L'쒠' < L'쒡'));
                CHECK((L'쒜' < letter) == (L'쒜' < L'쒡'));
                CHECK((L'쑤' < letter) == (L'쑤' < L'쒡'));
                CHECK((L'ㅆ' < letter) == (L'ㅆ' < L'쒡'));
                CHECK((L'쒢' < letter) == (L'쒢' < L'쒡'));
                CHECK((L't' < letter) == (L't' < L'쒡'));
                CHECK((L'T' < letter) == (L'T' < L'쒡'));
                CHECK((L'1' < letter) == (L'1' < L'쒡'));
                CHECK((L'.' < letter) == (L'.' < L'쒡'));
            }
        }

        SUBCASE("특수 문자")
        {
            CHECK((Letter{ Letter::NON_WORD_LETTER } < L'a') == true );
            CHECK((L'a' < Letter{ Letter::NON_WORD_LETTER }) == false);

            CHECK((Letter{ Letter::NON_WORD_LETTER } < L'A') == true );
            CHECK((L'A' < Letter{ Letter::NON_WORD_LETTER }) == false);


            CHECK((Letter{ Letter::NON_WORD_LETTER } < L'1') == true );
            CHECK((L'1' < Letter{ Letter::NON_WORD_LETTER }) == false);


            CHECK((Letter{ Letter::NON_WORD_LETTER } < L'.') == true );
            CHECK((L'.' < Letter{ Letter::NON_WORD_LETTER }) == false);


            CHECK((Letter{ Letter::NON_WORD_LETTER } < L'ㅁ') == true );
            CHECK((L'ㅁ' < Letter{ Letter::NON_WORD_LETTER }) == false);
        }

        SUBCASE("equal_range")
        {
            Letter letters[] = {
                Letter{ L'a', false },
                Letter{ L'a', true },  Letter{ L'A', true },

                Letter{ L'b', false },
                Letter{ L'b', true },  Letter{ L'B', true },

                Letter{ L'1' }, Letter{ L'.' }, Letter{ L' ' },
                Letter{ L'{' }, Letter{ L'|' }, Letter{ L'~' },
                Letter{ L'ㄱ' }, Letter{ L'나' }, Letter{ L'ㅢ' }, Letter{ L'쒡' },

                Letter{ Letter::NON_WORD_LETTER },
            };
            std::sort(std::begin(letters), std::end(letters));

            constexpr wchar_t chars[] = {
                L'a', L'A', L'b', L'B',
                L'1', L'.', L' ', L'{', L'|', L'~',
                L'ㄱ', L'나', L'ㅢ', L'쒡',
            };

            for (const wchar_t ch : chars)
            {
                for (const Letter& letter : letters)
                {
                    const bool a = letter < ch;
                    const bool b = !(ch < letter);
                    const bool check = !a || b;
                    CHECK(check);
                }
                CHECK(std::ranges::is_partitioned(letters, [ch](const Letter& letter) { return letter < ch; }));
                CHECK(std::ranges::is_partitioned(letters, [ch](const Letter& letter) { return !(ch < letter); }));
            }

            {
                const auto [beg, end] = std::equal_range(std::begin(letters), std::end(letters), L'a');
                std::span<const Letter> range{ beg, end };
                REQUIRE(range.size() == 2);
                CHECK(range[0] == Letter{ L'a', false });
                CHECK(range[1] == Letter{ L'a', true  });
            }

            {
                const auto [beg, end] = std::equal_range(std::begin(letters), std::end(letters), L'A');
                std::span<const Letter> range{ beg, end };
                REQUIRE(range.size() == 2);
                CHECK(range[0] == Letter{ L'A', true  });
                CHECK(range[1] == Letter{ L'A', false });
            }

            {
                const auto [beg, end] = std::equal_range(std::begin(letters), std::end(letters), L'1');
                std::span<const Letter> range{ beg, end };
                REQUIRE(range.size() == 1);
                CHECK(range[0] == Letter{ L'1' });
            }

            {
                const auto [beg, end] = std::equal_range(std::begin(letters), std::end(letters), L'|');
                std::span<const Letter> range{ beg, end };
                REQUIRE(range.size() == 1);
                CHECK(range[0] == Letter{ L'|' });
            }

            {
                const auto [beg, end] = std::equal_range(std::begin(letters), std::end(letters), L'쒡');
                std::span<const Letter> range{ beg, end };
                REQUIRE(range.size() == 1);
                CHECK(range[0] == Letter{ L'쒡' });
            }
        }
    }

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
}
