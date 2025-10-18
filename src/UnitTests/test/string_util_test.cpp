#include <doctest.h>

#include "../util/test_util.h"


TEST_SUITE("String Util")
{
    TEST_CASE("Normalize Hangeul")
    {
        SUBCASE("Non-Hangeul")
        {
            check_normalization(L"abcXYZ012,./*() \n\t", L"abcXYZ012,./*() \n\t");
            check_normalization(L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮", L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮");
        }

        SUBCASE("Hangeul - Incomplete Single Letter Composite")
        {
            check_normalization(L"ㄱㄴㄷㄹㅁㅂㅅㅇㅈㅊㅋㅌㅍㅎ", L"ㄱㄴㄷㄹㅁㅂㅅㅇㅈㅊㅋㅌㅍㅎ");
            check_normalization(L"ㅏㅑㅓㅕㅗㅛㅜㅠㅡㅣㅐㅔ", L"ㅏㅑㅓㅕㅗㅛㅜㅠㅡㅣㅐㅔ");
            check_normalization(L"ㄲㄸㅃㅆㅉㅒㅖ", L"ㄲㄸㅃㅆㅉㅒㅖ");
        }

        SUBCASE("Hangeul - Single Letter Vowel")
        {
            check_normalization(L"가냐더려모뵤수유즈치캐테파햐", L"ㄱㅏㄴㅑㄷㅓㄹㅕㅁㅗㅂㅛㅅㅜㅇㅠㅈㅡㅊㅣㅋㅐㅌㅔㅍㅏㅎㅑ");
            check_normalization(L"꺠뗴빠쌰쩌걔녜", L"ㄲㅒㄸㅖㅃㅏㅆㅑㅉㅓㄱㅒㄴㅖ");
        }

        SUBCASE("Hangeul - Double Letter Vowel")
        {
            check_normalization(L"ㅘㅙㅚㅝㅞㅟㅢ", L"ㅗㅏㅗㅐㅗㅣㅜㅓㅜㅔㅜㅣㅡㅣ");
            check_normalization(L"꽈뙈뾔쒀쮀귀늬", L"ㄲㅗㅏㄸㅗㅐㅃㅗㅣㅆㅜㅓㅉㅜㅔㄱㅜㅣㄴㅡㅣ");
        }

        SUBCASE("Hangeul - Single Letter Final")
        {
            check_normalization(L"각꺆놘뙏럘뼴법셧옸죵춪큧틐핕힆쒷", L"ㄱㅏㄱㄲㅑㄲㄴㅗㅏㄴㄸㅗㅐㄷㄹㅒㄹㅃㅖㅁㅂㅓㅂㅅㅕㅅㅇㅗㅆㅈㅛㅇㅊㅜㅈㅋㅠㅊㅌㅡㅋㅍㅣㅌㅎㅡㅣㅍㅆㅜㅔㅎ");
        }

        SUBCASE("Hangeul - Double Letter Final")
        {
            check_normalization(L"ㄳㄵㄶㄺㄻㄼㄽㄾㄿㅀㅄ", L"ㄱㅅㄴㅈㄴㅎㄹㄱㄹㅁㄹㅂㄹㅅㄹㅌㄹㅍㄹㅎㅂㅅ");
            check_normalization(L"갃꺉놚뙑럚뼯벐셡옲죯춦", L"ㄱㅏㄱㅅㄲㅑㄴㅈㄴㅗㅏㄴㅎㄸㅗㅐㄹㄱㄹㅒㄹㅁㅃㅖㄹㅂㅂㅓㄹㅅㅅㅕㄹㅌㅇㅗㄹㅍㅈㅛㄹㅎㅊㅜㅂㅅ");
        }
    }
}
