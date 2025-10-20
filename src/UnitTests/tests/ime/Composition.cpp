#include <doctest.h>

#include <string_view>

#include "Typoon/ime/Composition.h"

#include "util/DocTestWStringSupport.h"


namespace
{

void check_compose_letter(const std::wstring_view alphabets, const wchar_t expected)
{
    // 이 subcase에서는 ComposeLetter() 함수만을 확인할 것이기 때문에 조합된 글자가 출력되는 일이 없어야 함.
    const auto lambdaErrorOnCallback = [](wchar_t) { REQUIRE(false); };
    Composition composition{ lambdaErrorOnCallback };

    for (const wchar_t alphabet : alphabets)
    {
        composition.AddAlphabet(alphabet);
    }

    CHECK(composition.ComposeLetter() == expected);
}

void check_add_alphabet(const std::wstring_view alphabets, const std::wstring_view expected)
{
    std::wstring result;

    Composition composition{ [&result](const wchar_t c) { result.push_back(c); } };

    for (const wchar_t alphabet : alphabets)
    {
        composition.AddAlphabet(alphabet);
    }
    if (const wchar_t lastLetter = composition.ComposeLetter();
        lastLetter != 0)
    {
        result.push_back(lastLetter);
    }

    CHECK(result == expected);
}

}


TEST_SUITE("IME")
{
    TEST_CASE("Composition")
    {
        SUBCASE("ComposeLetter() Sanity Check")
        {
            /// 초성
            check_compose_letter(L"ㄱ", L'ㄱ');
            check_compose_letter(L"ㄲ", L'ㄲ');
            check_compose_letter(L"ㅆ", L'ㅆ');
            check_compose_letter(L"ㅎ", L'ㅎ');

            /// 중성
            check_compose_letter(L"ㅏ", L'ㅏ');
            check_compose_letter(L"ㅔ", L'ㅔ');
            check_compose_letter(L"ㅗㅏ", L'ㅘ');
            check_compose_letter(L"ㅜㅔ", L'ㅞ');

            /// 종성
            check_compose_letter(L"ㄱㅅ", L'ㄳ');
            check_compose_letter(L"ㄴㅎ", L'ㄶ');
            check_compose_letter(L"ㅂㅅ", L'ㅄ');
            
            /// 초성 + 중성
            check_compose_letter(L"ㄱㅏ", L'가');
            check_compose_letter(L"ㄲㅏ", L'까');
            check_compose_letter(L"ㄲㅐ", L'깨');
            check_compose_letter(L"ㄲㅗㅏ", L'꽈');
            check_compose_letter(L"ㄲㅗㅐ", L'꽤');

            /// 초성 + 중성 + 종성
            check_compose_letter(L"ㅅㅏㅅ", L'삿');
            check_compose_letter(L"ㅆㅏㅅ", L'쌋');
            check_compose_letter(L"ㅆㅐㅅ", L'쌧');
            check_compose_letter(L"ㅆㅗㅏㅅ", L'쐇');
            check_compose_letter(L"ㅆㅗㅐㅅ", L'쐣');
            check_compose_letter(L"ㅆㅗㅐㅆ", L'쐤');
            check_compose_letter(L"ㅆㅗㅐㄱㅅ", L'쐓');
        }

        SUBCASE("AddAlphabet() - 순수 한글만")
        {
            // 초성
            check_add_alphabet(L"ㄱㄴㄷㄹㅇㅁㅂㅈㅅㅊㅋㅌㅍㅎㄲㄸㅃㅆㅉ", L"ㄱㄴㄷㄹㅇㅁㅂㅈㅅㅊㅋㅌㅍㅎㄲㄸㅃㅆㅉ");

            // 중성
            check_add_alphabet(L"ㅏㅑㅓㅕㅗㅛㅜㅠㅣㅡㅐㅔㅗㅏㅗㅐㅗㅣㅜㅓㅜㅔㅜㅣㅡㅣ", L"ㅏㅑㅓㅕㅗㅛㅜㅠㅣㅡㅐㅔㅘㅙㅚㅝㅞㅟㅢ");

            // 종성
            check_add_alphabet(L"ㄱㅅㄴㅈㄴㅎㄹㄱㄹㅁㄹㅂㄹㅅㄹㅌㄹㅍㄹㅎㅂㅅ", L"ㄳㄵㄶㄺㄻㄼㄽㄾㄿㅀㅄ");

            // 종성 없는 문장
            check_add_alphabet(L"ㅋㅏㄹㅔㄱㅏㅉㅏㅇㅛㅇㅗㅁㅡㄹㅏㅇㅣㅅㅡㄱㅏㅉㅏㅇㅛㅇㅏㅂㅓㅈㅣㅇㅗㅏㅇㅓㅁㅓㄴㅣㄱㅏㅂㅏㄷㅏㄹㅗㅎㅠㄱㅏㄱㅏㅇㅛ", 
                L"카레가짜요오므라이스가짜요아버지와어머니가바다로휴가가요");

            // 일반 문장
            check_add_alphabet(
                L"ㅁㅗㄷㅡㄴㄱㅜㄱㅁㅣㄴㅇㅡㄴㅅㅏㅅㅐㅇㅎㅗㅏㄹㅇㅡㅣㅂㅣㅁㅣㄹㄱㅗㅏㅈㅏㅇㅠㄹㅡㄹㅊㅣㅁㅎㅐㅂㅏㄷㅈㅣㅇㅏㄴㅣㅎㅏㄴㄷㅏ"
                "ㅅㅏㅂㅓㅂㄱㅜㅓㄴㅇㅡㄴㅂㅓㅂㄱㅗㅏㄴㅇㅡㄹㅗㄱㅜㅅㅓㅇㄷㅗㅣㄴㅂㅓㅂㅇㅜㅓㄴㅇㅔㅅㅗㄱㅎㅏㄴㄷㅏ"
                "ㄴㅜㄱㅜㄷㅡㄴㅈㅣㅊㅔㅍㅗㄸㅗㄴㅡㄴㄱㅜㅅㅗㄱㅇㅡㅣㅇㅣㅇㅠㅇㅗㅏㅂㅕㄴㅎㅗㅇㅣㄴㅇㅡㅣㅈㅗㄹㅕㄱㅇㅡㄹㅂㅏㄷㅇㅡㄹㄱㅜㅓㄴㄹㅣㄱㅏㅇㅣㅆㅇㅡㅁㅇㅡㄹㄱㅗㅈㅣㅂㅏㄷㅈㅣㅇㅏㄴㅣㅎㅏㄱㅗㄴㅡㄴㅊㅔㅍㅗㄸㅗㄴㅡㄴㄱㅜㅅㅗㄱㅇㅡㄹㄷㅏㅇㅎㅏㅈㅣㅇㅏㄴㅣㅎㅏㄴㄷㅏ"
                "ㅊㅔㅍㅗㄸㅗㄴㅡㄴㄱㅜㅅㅗㄱㅇㅡㄹㄷㅏㅇㅎㅏㄴㅈㅏㅇㅡㅣㄱㅏㅈㅗㄱㄷㅡㅇㅂㅓㅂㄹㅠㄹㅇㅣㅈㅓㅇㅎㅏㄴㅡㄴㅈㅏㅇㅔㄱㅔㄴㅡㄴㄱㅡㅇㅣㅇㅠㅇㅗㅏㅇㅣㄹㅅㅣㅈㅏㅇㅅㅗㄱㅏㅈㅣㅊㅔㅇㅓㅂㅅㅇㅣㅌㅗㅇㅈㅣㄷㅗㅣㅇㅓㅇㅑㅎㅏㄴㄷㅏ"
                "ㄷㅐㅎㅏㄴㅁㅣㄴㄱㅜㄱㅇㅡㅣㅈㅜㄱㅜㅓㄴㅇㅡㄴㄱㅜㄱㅁㅣㄴㅇㅔㄱㅔㅇㅣㅆㄱㅗㅁㅗㄷㅡㄴㄱㅜㅓㄴㄹㅕㄱㅇㅡㄴㄱㅜㄱㅁㅣㄴㅇㅡㄹㅗㅂㅜㅌㅓㄴㅏㅇㅗㄴㄷㅏ"
                "ㄱㅜㄱㅁㅜㅎㅗㅣㅇㅡㅣㄴㅡㄴㅈㅓㅇㅂㅜㅇㅡㅣㄱㅜㅓㄴㅎㅏㄴㅇㅔㅅㅗㄱㅎㅏㄴㅡㄴㅈㅜㅇㅇㅛㅎㅏㄴㅈㅓㅇㅊㅐㄱㅇㅡㄹㅅㅣㅁㅇㅡㅣㅎㅏㄴㄷㅏ"
                "ㅈㅓㄴㅈㅣㄱㄷㅐㅌㅗㅇㄹㅕㅇㅇㅡㅣㅅㅣㄴㅂㅜㄴㄱㅗㅏㅇㅖㅇㅜㅇㅔㄱㅗㅏㄴㅎㅏㅇㅕㄴㅡㄴㅂㅓㅂㄹㅠㄹㄹㅗㅈㅓㅇㅎㅏㄴㄷㅏ",
                L"모든국민은사생활의비밀과자유를침해받지아니한다"
                "사법권은법관으로구성된법원에속한다"
                "누구든지체포또는구속의이유와변호인의조력을받을권리가있음을고지받지아니하고는체포또는구속을당하지아니한다"
                "체포또는구속을당한자의가족등법률이정하는자에게는그이유와일시장소가지체없이통지되어야한다"
                "대한민국의주권은국민에게있고모든권력은국민으로부터나온다"
                "국무회의는정부의권한에속하는중요한정책을심의한다"
                "전직대통령의신분과예우에관하여는법률로정한다");
            check_add_alphabet(
                L"ㄱㅖㅈㅓㄹㅇㅣㅈㅣㄴㅏㄱㅏㄴㅡㄴㅎㅏㄴㅡㄹㅇㅔㄴㅡㄴ"
                "ㄱㅏㅇㅡㄹㄹㅗㄱㅏㄷㅡㄱㅊㅏㅇㅣㅆㅅㅡㅂㄴㅣㄷㅏ"
                "ㄴㅏㄴㅡㄴㅇㅏㅁㅜㄱㅓㄱㅈㅓㅇㄷㅗㅇㅓㅂㅅㅇㅣ"
                "ㄱㅏㅇㅡㄹㅅㅗㄱㅇㅡㅣㅂㅕㄹㄷㅡㄹㅇㅡㄹㄷㅏㅎㅔㅇㅣㄹㄷㅡㅅㅎㅏㅂㄴㅣㄷㅏ"
                "ㄱㅏㅅㅡㅁㅅㅗㄱㅇㅔㅎㅏㄴㅏㄷㅜㄹㅅㅐㄱㅕㅈㅣㄴㅡㄴㅂㅕㄹㅇㅡㄹ"
                "ㅇㅣㅈㅔㄷㅏㅁㅗㅅㅎㅔㄴㅡㄴㄱㅓㅅㅇㅡㄴ"
                "ㅅㅜㅣㅇㅣㅇㅏㅊㅣㅁㅇㅣㅇㅗㄴㅡㄴㄲㅏㄷㅏㄹㄱㅇㅣㅇㅛ"
                "ㄴㅐㅇㅣㄹㅂㅏㅁㅇㅣㄴㅏㅁㅇㅡㄴㄲㅏㄷㅏㄹㄱㅇㅣㅇㅛ"
                "ㅇㅏㅈㅣㄱㄴㅏㅇㅡㅣㅊㅓㅇㅊㅜㄴㅇㅣㄷㅏㅎㅏㅈㅣㅇㅏㄴㅎㅇㅡㄴㄲㅏㄷㅏㄹㄱㅇㅣㅂㄴㅣㄷㅏ",
                L"계절이지나가는하늘에는"
                "가을로가득차있습니다"
                "나는아무걱정도없이"
                "가을속의별들을다헤일듯합니다"
                "가슴속에하나둘새겨지는별을"
                "이제다못헤는것은"
                "쉬이아침이오는까닭이요"
                "내일밤이남은까닭이요"
                "아직나의청춘이다하지않은까닭입니다");
        }

        SUBCASE("AddAlphabet() - 한글이 아닌 문자 포함")
        {
            // 종성 없는 문장
            check_add_alphabet(L"ㅋㅏㄹㅔㄱㅏ ㅉㅏㅇㅛ. ㅇㅗㅁㅡㄹㅏㅇㅣㅅㅡㄱㅏ ㅉㅏㅇㅛ. ㅇㅏㅂㅓㅈㅣㅇㅗㅏ ㅇㅓㅁㅓㄴㅣㄱㅏ ㅂㅏㄷㅏㄹㅗ ㅎㅠㄱㅏ ㄱㅏㅇㅛ.",
                L"카레가 짜요. 오므라이스가 짜요. 아버지와 어머니가 바다로 휴가 가요.");

            // 일반 문장
            check_add_alphabet(
                L"ㅁㅗㄷㅡㄴ ㄱㅜㄱㅁㅣㄴㅇㅡㄴ ㅅㅏㅅㅐㅇㅎㅗㅏㄹㅇㅡㅣ ㅂㅣㅁㅣㄹㄱㅗㅏ ㅈㅏㅇㅠㄹㅡㄹ ㅊㅣㅁㅎㅐㅂㅏㄷㅈㅣ ㅇㅏㄴㅣㅎㅏㄴㄷㅏ. "
                "ㅅㅏㅂㅓㅂㄱㅜㅓㄴㅇㅡㄴ ㅂㅓㅂㄱㅗㅏㄴㅇㅡㄹㅗ ㄱㅜㅅㅓㅇㄷㅗㅣㄴ ㅂㅓㅂㅇㅜㅓㄴㅇㅔ ㅅㅗㄱㅎㅏㄴㄷㅏ. "
                "ㄴㅜㄱㅜㄷㅡㄴㅈㅣ ㅊㅔㅍㅗ ㄸㅗㄴㅡㄴ ㄱㅜㅅㅗㄱㅇㅡㅣ ㅇㅣㅇㅠㅇㅗㅏ ㅂㅕㄴㅎㅗㅇㅣㄴㅇㅡㅣ ㅈㅗㄹㅕㄱㅇㅡㄹ ㅂㅏㄷㅇㅡㄹ ㄱㅜㅓㄴㄹㅣㄱㅏ ㅇㅣㅆㅇㅡㅁㅇㅡㄹ ㄱㅗㅈㅣㅂㅏㄷㅈㅣ ㅇㅏㄴㅣㅎㅏㄱㅗㄴㅡㄴ ㅊㅔㅍㅗ ㄸㅗㄴㅡㄴ ㄱㅜㅅㅗㄱㅇㅡㄹ ㄷㅏㅇㅎㅏㅈㅣ ㅇㅏㄴㅣㅎㅏㄴㄷㅏ. "
                "ㅊㅔㅍㅗ ㄸㅗㄴㅡㄴ ㄱㅜㅅㅗㄱㅇㅡㄹ ㄷㅏㅇㅎㅏㄴ ㅈㅏㅇㅡㅣ ㄱㅏㅈㅗㄱㄷㅡㅇ ㅂㅓㅂㄹㅠㄹㅇㅣ ㅈㅓㅇㅎㅏㄴㅡㄴ ㅈㅏㅇㅔㄱㅔㄴㅡㄴ ㄱㅡ ㅇㅣㅇㅠㅇㅗㅏ ㅇㅣㄹㅅㅣ·ㅈㅏㅇㅅㅗㄱㅏ ㅈㅣㅊㅔㅇㅓㅂㅅㅇㅣ ㅌㅗㅇㅈㅣㄷㅗㅣㅇㅓㅇㅑ ㅎㅏㄴㄷㅏ. "
                "ㄷㅐㅎㅏㄴㅁㅣㄴㄱㅜㄱㅇㅡㅣ ㅈㅜㄱㅜㅓㄴㅇㅡㄴ ㄱㅜㄱㅁㅣㄴㅇㅔㄱㅔ ㅇㅣㅆㄱㅗ, ㅁㅗㄷㅡㄴ ㄱㅜㅓㄴㄹㅕㄱㅇㅡㄴ ㄱㅜㄱㅁㅣㄴㅇㅡㄹㅗㅂㅜㅌㅓ ㄴㅏㅇㅗㄴㄷㅏ. "
                "ㄱㅜㄱㅁㅜㅎㅗㅣㅇㅡㅣㄴㅡㄴ ㅈㅓㅇㅂㅜㅇㅡㅣ ㄱㅜㅓㄴㅎㅏㄴㅇㅔ ㅅㅗㄱㅎㅏㄴㅡㄴ ㅈㅜㅇㅇㅛㅎㅏㄴ ㅈㅓㅇㅊㅐㄱㅇㅡㄹ ㅅㅣㅁㅇㅡㅣㅎㅏㄴㄷㅏ. "
                "ㅈㅓㄴㅈㅣㄱㄷㅐㅌㅗㅇㄹㅕㅇㅇㅡㅣ ㅅㅣㄴㅂㅜㄴㄱㅗㅏ ㅇㅖㅇㅜㅇㅔ ㄱㅗㅏㄴㅎㅏㅇㅕㄴㅡㄴ ㅂㅓㅂㄹㅠㄹㄹㅗ ㅈㅓㅇㅎㅏㄴㄷㅏ.",
                L"모든 국민은 사생활의 비밀과 자유를 침해받지 아니한다. "
                "사법권은 법관으로 구성된 법원에 속한다. "
                "누구든지 체포 또는 구속의 이유와 변호인의 조력을 받을 권리가 있음을 고지받지 아니하고는 체포 또는 구속을 당하지 아니한다. "
                "체포 또는 구속을 당한 자의 가족등 법률이 정하는 자에게는 그 이유와 일시·장소가 지체없이 통지되어야 한다. "
                "대한민국의 주권은 국민에게 있고, 모든 권력은 국민으로부터 나온다. "
                "국무회의는 정부의 권한에 속하는 중요한 정책을 심의한다. "
                "전직대통령의 신분과 예우에 관하여는 법률로 정한다.");
            check_add_alphabet(
                LR"(季節ㅇㅣ ㅈㅣㄴㅏㄱㅏㄴㅡㄴ ㅎㅏㄴㅡㄹㅇㅔㄴㅡㄴ
                ㄱㅏㅇㅡㄹㄹㅗ ㄱㅏㄷㅡㄱ ㅊㅏㅇㅣㅆㅅㅡㅂㄴㅣㄷㅏ。

                ㄴㅏㄴㅡㄴ ㅇㅏㅁㅜ ㄱㅓㄱㅈㅓㅇㄷㅗ ㅇㅓㅂㅅㅇㅣ
                ㄱㅏㅇㅡㄹㅅㅗㄱㅇㅡㅣ ㅂㅕㄹㄷㅡㄹㅇㅡㄹ ㄷㅏ ㅎㅔㅇㅣㄹㄷㅡㅅㅎㅏㅂㄴㅣㄷㅏ。

                ㄱㅏㅅㅡㅁㅅㅗㄱㅇㅔ ㅎㅏㄴㅏ ㄷㅜㄹ ㅅㅐㄱㅇㅕㅈㅣㄴㅡㄴ ㅂㅕㄹㅇㅡㄹ
                ㅇㅣㅈㅔ ㄷㅏ ㅁㅗㅅㅎㅔㄴㅡㄴ ㄱㅓㅅㅇㅡㄴ
                ㅅㅜㅣㅇㅣ ㅇㅏㅊㅡㅁㅇㅣ ㅇㅗㄴㅡㄴ ㄲㅏㄷㅏㄹㄱㅇㅣㅇㅗ、
                來日ㅂㅏㅁㅇㅣ ㄴㅏㅁㅇㅡㄴ ㄲㅏㄷㅏㄹㄱㅇㅣㅇㅗ、
                ㅇㅏㅈㅣㄱ ㄴㅏㅇㅡㅣ 靑春ㅇㅣ ㄷㅏㅎㅏㅈㅣ ㅇㅏㄴㅎㅇㅡㄴ ㄲㅏㄷㅏㄹㄱㅇㅣㅂㄴㅣㄷㅏ。)",
                LR"(季節이 지나가는 하늘에는
                가을로 가득 차있습니다。

                나는 아무 걱정도 없이
                가을속의 별들을 다 헤일듯합니다。

                가슴속에 하나 둘 색여지는 별을
                이제 다 못헤는 것은
                쉬이 아츰이 오는 까닭이오、
                來日밤이 남은 까닭이오、
                아직 나의 靑春이 다하지 않은 까닭입니다。)");
        }

        SUBCASE("AddAlphabet() - 백스페이스")
        {
            bool shouldOutputBackspace = false;
            const auto lambdaCheckBackspace =
                [&shouldOutputBackspace](const wchar_t ch)
                {
                    CHECK(ch == L'\b');
                    REQUIRE(shouldOutputBackspace == true);
                    shouldOutputBackspace = false;
                };
            Composition composition{ lambdaCheckBackspace };

            shouldOutputBackspace = true;
            composition.AddAlphabet(L'\b');

            composition.AddAlphabet(L'ㄱ');
            composition.AddAlphabet(L'ㅅ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'ㄱ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'\0');
            shouldOutputBackspace = true;
            composition.AddAlphabet(L'\b');

            composition.AddAlphabet(L'ㄱ');
            composition.AddAlphabet(L'ㅏ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'ㄱ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'\0');
            shouldOutputBackspace = true;
            composition.AddAlphabet(L'\b');

            composition.AddAlphabet(L'ㄱ');
            composition.AddAlphabet(L'ㅗ');
            composition.AddAlphabet(L'ㅏ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'고');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'ㄱ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'\0');
            shouldOutputBackspace = true;
            composition.AddAlphabet(L'\b');

            composition.AddAlphabet(L'ㄱ');
            composition.AddAlphabet(L'ㅗ');
            composition.AddAlphabet(L'ㅏ');
            composition.AddAlphabet(L'ㄴ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'과');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'고');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'ㄱ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'\0');
            shouldOutputBackspace = true;
            composition.AddAlphabet(L'\b');

            composition.AddAlphabet(L'ㄱ');
            composition.AddAlphabet(L'ㅗ');
            composition.AddAlphabet(L'ㅏ');
            composition.AddAlphabet(L'ㄴ');
            composition.AddAlphabet(L'ㅈ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'관');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'과');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'고');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'ㄱ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'\0');
            shouldOutputBackspace = true;
            composition.AddAlphabet(L'\b');

            composition.AddAlphabet(L'ㅗ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'\0');
            shouldOutputBackspace = true;
            composition.AddAlphabet(L'\b');

            composition.AddAlphabet(L'ㅗ');
            composition.AddAlphabet(L'ㅏ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'ㅗ');
            composition.AddAlphabet(L'\b');
            CHECK(composition.ComposeLetter() == L'\0');
            shouldOutputBackspace = true;
            composition.AddAlphabet(L'\b');
        }
    }
}
