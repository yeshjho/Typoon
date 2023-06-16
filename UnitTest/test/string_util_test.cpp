#include <doctest.h>

#include "../../Typoon/utils/string.h"
#include "../util/test_util.h"


TEST_SUITE("String Util")
{
    TEST_CASE("Normalize Hangeul")
    {
        SUBCASE("Non-Hangeul")
        {
            CHECK(normalize_hangeul(L"abcXYZ012,./*() \n\t") == L"abcXYZ012,./*() \n\t");
            CHECK(normalize_hangeul(L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮") == L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮");
        }

        SUBCASE("Hangeul - Incomplete Single Letter Composite")
        {
            CHECK(normalize_hangeul(L"ㄱㄴㄷㄹㅁㅂㅅㅇㅈㅊㅋㅌㅍㅎ") == L"ㄱㄴㄷㄹㅁㅂㅅㅇㅈㅊㅋㅌㅍㅎ");
            CHECK(normalize_hangeul(L"ㅏㅑㅓㅕㅗㅛㅜㅠㅡㅣㅐㅔ") == L"ㅏㅑㅓㅕㅗㅛㅜㅠㅡㅣㅐㅔ");
            CHECK(normalize_hangeul(L"ㄲㄸㅃㅆㅉㅒㅖ") == L"ㄲㄸㅃㅆㅉㅒㅖ");
        }

        SUBCASE("Hangeul - Single Letter Vowel")
        {
            CHECK(normalize_hangeul(L"가냐더려모뵤수유즈치캐테파햐") == L"ㄱㅏㄴㅑㄷㅓㄹㅕㅁㅗㅂㅛㅅㅜㅇㅠㅈㅡㅊㅣㅋㅐㅌㅔㅍㅏㅎㅑ");
            CHECK(normalize_hangeul(L"꺠뗴빠쌰쩌걔녜") == L"ㄲㅒㄸㅖㅃㅏㅆㅑㅉㅓㄱㅒㄴㅖ");
        }

        SUBCASE("Hangeul - Double Letter Vowel")
        {
            CHECK(normalize_hangeul(L"ㅘㅙㅚㅝㅞㅟㅢ") == L"ㅗㅏㅗㅐㅗㅣㅜㅓㅜㅔㅜㅣㅡㅣ");
            CHECK(normalize_hangeul(L"꽈뙈뾔쒀쮀귀늬") == L"ㄲㅗㅏㄸㅗㅐㅃㅗㅣㅆㅜㅓㅉㅜㅔㄱㅜㅣㄴㅡㅣ");
        }

        SUBCASE("Hangeul - Single Letter Final")
        {
            CHECK(normalize_hangeul(L"각꺆놘뙏럘뼴법셧옸죵춪큧틐핕힆쒷") == L"ㄱㅏㄱㄲㅑㄲㄴㅗㅏㄴㄸㅗㅐㄷㄹㅒㄹㅃㅖㅁㅂㅓㅂㅅㅕㅅㅇㅗㅆㅈㅛㅇㅊㅜㅈㅋㅠㅊㅌㅡㅋㅍㅣㅌㅎㅡㅣㅍㅆㅜㅔㅎ");
        }

        SUBCASE("Hangeul - Double Letter Final")
        {
            CHECK(normalize_hangeul(L"ㄳㄵㄶㄺㄻㄼㄽㄾㄿㅀㅄ") == L"ㄱㅅㄴㅈㄴㅎㄹㄱㄹㅁㄹㅂㄹㅅㄹㅌㄹㅍㄹㅎㅂㅅ");
            CHECK(normalize_hangeul(L"갃꺉놚뙑럚뼯벐셡옲죯춦") == L"ㄱㅏㄱㅅㄲㅑㄴㅈㄴㅗㅏㄴㅎㄸㅗㅐㄹㄱㄹㅒㄹㅁㅃㅖㄹㅂㅂㅓㄹㅅㅅㅕㄹㅌㅇㅗㄹㅍㅈㅛㄹㅎㅊㅜㅂㅅ");
        }
    }
}
