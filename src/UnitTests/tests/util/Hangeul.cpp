#include <doctest.h>

#include "Typoon/util/Hangeul.h"

#include "util/DocTestWStringSupport.h"


using namespace typoon::util;

TEST_SUITE("Hangeul")
{
    TEST_CASE("decompose_hangeul")
    {
        SUBCASE("한글이 아닌 문자")
        {
            CHECK(decompose_hangeul(L"abcXYZ012,./*() \n\t") == L"abcXYZ012,./*() \n\t");
            CHECK(decompose_hangeul(L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮") == L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮");
        }

        SUBCASE("한글 자모")
        {
            CHECK(decompose_hangeul(L"ㄱㄴㄷㄹㅁㅂㅅㅇㅈㅊㅋㅌㅍㅎ") == L"ㄱㄴㄷㄹㅁㅂㅅㅇㅈㅊㅋㅌㅍㅎ");
            CHECK(decompose_hangeul(L"ㅏㅑㅓㅕㅗㅛㅜㅠㅡㅣㅐㅔ") == L"ㅏㅑㅓㅕㅗㅛㅜㅠㅡㅣㅐㅔ");
            CHECK(decompose_hangeul(L"ㄲㄸㅃㅆㅉㅒㅖ") == L"ㄲㄸㅃㅆㅉㅒㅖ");
        }

        SUBCASE("한글 한 글자 모음")
        {
            CHECK(decompose_hangeul(L"가냐더려모뵤수유즈치캐테파햐") == L"ㄱㅏㄴㅑㄷㅓㄹㅕㅁㅗㅂㅛㅅㅜㅇㅠㅈㅡㅊㅣㅋㅐㅌㅔㅍㅏㅎㅑ");
            CHECK(decompose_hangeul(L"꺠뗴빠쌰쩌걔녜") == L"ㄲㅒㄸㅖㅃㅏㅆㅑㅉㅓㄱㅒㄴㅖ");
        }

        SUBCASE("한글 두 글자 모음")
        {
            CHECK(decompose_hangeul(L"ㅘㅙㅚㅝㅞㅟㅢ") == L"ㅗㅏㅗㅐㅗㅣㅜㅓㅜㅔㅜㅣㅡㅣ");
            CHECK(decompose_hangeul(L"꽈뙈뾔쒀쮀귀늬") == L"ㄲㅗㅏㄸㅗㅐㅃㅗㅣㅆㅜㅓㅉㅜㅔㄱㅜㅣㄴㅡㅣ");
        }

        SUBCASE("한글 한 글자 받침")
        {
            CHECK(decompose_hangeul(L"각꺆놘뙏럘뼴법셧옸죵춪큧틐핕힆쒷") == L"ㄱㅏㄱㄲㅑㄲㄴㅗㅏㄴㄸㅗㅐㄷㄹㅒㄹㅃㅖㅁㅂㅓㅂㅅㅕㅅㅇㅗㅆㅈㅛㅇㅊㅜㅈㅋㅠㅊㅌㅡㅋㅍㅣㅌㅎㅡㅣㅍㅆㅜㅔㅎ");
        }

        SUBCASE("한글 겹받침")
        {
            CHECK(decompose_hangeul(L"ㄳㄵㄶㄺㄻㄼㄽㄾㄿㅀㅄ") == L"ㄱㅅㄴㅈㄴㅎㄹㄱㄹㅁㄹㅂㄹㅅㄹㅌㄹㅍㄹㅎㅂㅅ");
            CHECK(decompose_hangeul(L"갃꺉놚뙑럚뼯벐셡옲죯춦") == L"ㄱㅏㄱㅅㄲㅑㄴㅈㄴㅗㅏㄴㅎㄸㅗㅐㄹㄱㄹㅒㄹㅁㅃㅖㄹㅂㅂㅓㄹㅅㅅㅕㄹㅌㅇㅗㄹㅍㅈㅛㄹㅎㅊㅜㅂㅅ");
        }
    }

    TEST_CASE("combine_hangeul")
    {
        SUBCASE("한글이 아닌 문자")
        {
            CHECK(combine_hangeul(L"abcXYZ012,./*() \n\t") == L"abcXYZ012,./*() \n\t");
            CHECK(combine_hangeul(L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮") == L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮");
        }

        SUBCASE("한글 자모")
        {
            CHECK(combine_hangeul(L"ㅎㅍㅌㅋㅊㅈㅇㅅㅂㅁㄹㄷㄴㄱ") == L"ㅎㅍㅌㅋㅊㅈㅇㅅㅂㅁㄹㄷㄴㄱ");
            CHECK(combine_hangeul(L"ㅔㅐㅣㅡㅠㅜㅛㅗㅕㅓㅑㅏ") == L"ㅔㅐㅣㅡㅠㅜㅛㅗㅕㅓㅑㅏ");
            CHECK(combine_hangeul(L"ㅒㅖㄲㄸㅃㅆㅉ") == L"ㅒㅖㄲㄸㅃㅆㅉ");
        }

        SUBCASE("한글 한 글자 모음")
        {
            CHECK(combine_hangeul(L"ㄱㅏㄴㅑㄷㅓㄹㅕㅁㅗㅂㅛㅅㅜㅇㅠㅈㅡㅊㅣㅋㅐㅌㅔㅍㅏㅎㅑ") == L"가냐더려모뵤수유즈치캐테파햐");
            CHECK(combine_hangeul(L"ㄲㅒㄸㅖㅃㅏㅆㅑㅉㅓㄱㅒㄴㅖ") == L"꺠뗴빠쌰쩌걔녜");
        }

        SUBCASE("한글 두 글자 모음")
        {
            CHECK(combine_hangeul(L"ㅗㅏㅗㅐㅗㅣㅜㅓㅜㅔㅜㅣㅡㅣ") == L"ㅘㅙㅚㅝㅞㅟㅢ");
            CHECK(combine_hangeul(L"ㄲㅗㅏㄸㅗㅐㅃㅗㅣㅆㅜㅓㅉㅜㅔㄱㅜㅣㄴㅡㅣ") == L"꽈뙈뾔쒀쮀귀늬");
        }

        SUBCASE("한글 한 글자 받침")
        {
            CHECK(combine_hangeul(L"ㄱㅏㄱㄲㅑㄲㄴㅗㅏㄴㄸㅗㅐㄷㄹㅒㄹㅃㅖㅁㅂㅓㅂㅅㅕㅅㅇㅗㅆㅈㅛㅇㅊㅜㅈㅋㅠㅊㅌㅡㅋㅍㅣㅌㅎㅡㅣㅍㅆㅜㅔㅎ") == L"각꺆놘뙏럘뼴법셧옸죵춪큧틐핕힆쒷");
        }

        SUBCASE("한글 겹받침")
        {
            CHECK(combine_hangeul(L"ㄱㅅㄴㅈㄴㅎㄹㄱㄹㅁㄹㅂㄹㅅㄹㅌㄹㅍㄹㅎㅂㅅ") == L"ㄳㄵㄶㄺㄻㄼㄽㄾㄿㅀㅄ");
            CHECK(combine_hangeul(L"ㄱㅏㄱㅅㄲㅑㄴㅈㄴㅗㅏㄴㅎㄸㅗㅐㄹㄱㄹㅒㄹㅁㅃㅖㄹㅂㅂㅓㄹㅅㅅㅕㄹㅌㅇㅗㄹㅍㅈㅛㄹㅎㅊㅜㅂㅅ") == L"갃꺉놚뙑럚뼯벐셡옲죯춦");
        }
    }

    TEST_CASE("latin_alphabet_to_hangeul_alphabet")
    {
        SUBCASE("라틴 알파벳이 아닌 문자")
        {
            CHECK(latin_alphabet_to_hangeul_alphabet(L"ㄱㄴㄷㅏㅒㄲ012,./*() \n\t") == L"ㄱㄴㄷㅏㅒㄲ012,./*() \n\t");
            CHECK(latin_alphabet_to_hangeul_alphabet(L"áÁǒø") == L"áÁǒø");
            CHECK(latin_alphabet_to_hangeul_alphabet(L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮") == L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮");
        }

        SUBCASE("라틴 알파벳")
        {
            CHECK(latin_alphabet_to_hangeul_alphabet(L"abcdefghijklmnopqrstuvwxyz") == L"ㅁㅠㅊㅇㄷㄹㅎㅗㅑㅓㅏㅣㅡㅜㅐㅔㅂㄱㄴㅅㅕㅍㅈㅌㅛㅋ");
            CHECK(latin_alphabet_to_hangeul_alphabet(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ") == L"ㅁㅠㅊㅇㄸㄹㅎㅗㅑㅓㅏㅣㅡㅜㅒㅖㅃㄲㄴㅆㅕㅍㅉㅌㅛㅋ");
        }
    }

    TEST_CASE("hangeul_alphabet_to_latin_alphabet")
    {
        SUBCASE("한글이 아닌 문자")
        {
            CHECK(hangeul_alphabet_to_latin_alphabet(L"abcXYZ012,./*() \n\t") == L"abcXYZ012,./*() \n\t");
            CHECK(hangeul_alphabet_to_latin_alphabet(L"áÁǒø") == L"áÁǒø");
            CHECK(hangeul_alphabet_to_latin_alphabet(L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮") == L"ℾↁ⯶⼉⤵⿺ⰼ⢳⛚⡸♦⋀⁓ⷵ⌞ⓐ⃣ⴕ⻠⠉⻔⍆ⵦ⍳⏚⻭⟳╊≳⛠⓴◦⿍⭒☄↙⬣⟩ⵘ⁔Ⲏ‍╄ⓨ➗ ⅽ╷⹛✮");
        }

        SUBCASE("한글 자모")
        {
            CHECK(hangeul_alphabet_to_latin_alphabet(L"ㄱㄴㄷㄹㅁㅂㅅㅇㅈㅊㅋㅌㅍㅎㅏㅑㅓㅕㅗㅛㅜㅠㅡㅣㅐㅔ") == L"rsefaqtdwczxvgkijuhynbmlop");
            CHECK(hangeul_alphabet_to_latin_alphabet(L"ㅃㅉㄸㄲㅆㅒㅖ") == L"QWERTOP");
        }

        SUBCASE("결합된 자모")
        {
            CHECK(hangeul_alphabet_to_latin_alphabet(L"ㄳㄵㄻㅘㅝㅞ") == L"ㄳㄵㄻㅘㅝㅞ");
            CHECK(hangeul_alphabet_to_latin_alphabet(L"가낫똻") == L"가낫똻");
        }
    }
}
