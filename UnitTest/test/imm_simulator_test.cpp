#include <random>

#include <doctest.h>

#include "../../Typoon/imm/imm_simulator.h"
#include "../../Typoon/input_multicast/input_multicast.h"
#include "../../Typoon/utils/string.h"
#include "../util/test_util.h"


void imm_simulator_test(std::wstring_view text, std::wstring& result)
{
    simulate_type(normalize_hangeul(text));
    if (const wchar_t letter = imm_simulator.ComposeLetter();
        letter != 0)
    {
        result += letter;
    }

    CHECK(result == text);
}


TEST_SUITE("IMM Simulator")
{
    TEST_CASE("IMM Simulator - Basic")
    {
        setup_imm_simulator();

        std::wstring result;
        input_listeners.emplace_back("test",
            [&result](const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, [[maybe_unused]] bool clearAllAgents)
            {
                for (int i = 0; i < length; ++i)
                {
                    if (const auto [letter, isBeingComposed] = inputs[i];
                        !isBeingComposed)
                    {
                        result += letter;
                    }
                }
            }
        );

        SUBCASE("Initials Only")
        {
            imm_simulator_test(L"ㄱㄴㄷㄹㅇㅁㅂㅈㅅㅊㅋㅌㅍㅎㄲㄸㅃㅆㅉ", result);
        }

        SUBCASE("Medials only")
        {
            imm_simulator_test(L"ㅏㅑㅓㅕㅗㅛㅜㅠㅣㅡㅐㅔㅘㅙㅚㅝㅞㅟㅢ", result);
        }

        SUBCASE("Double Finals Only")
        {
            imm_simulator_test(L"ㄳㄵㄶㄺㄻㄼㄽㄾㄿㅀㅄ", result);
        }

        SUBCASE("No Finals")
        {
            imm_simulator_test(L"카레가 짜요. 오므라이스가 짜요. 아버지와 어머니가 바다로 휴가 가요.", result);
        }

        SUBCASE("Full Composite")
        {
            imm_simulator_test(LR"(모든 국민은 사생활의 비밀과 자유를 침해받지 아니한다. 사법권은 법관으로 구성된 법원에 속한다. 누구든지 체포 또는 구속의 이유와 변호인의 조력을 받을 권리가 있음을 고지받지 아니하고는 체포 또는 구속을 당하지 아니한다. 체포 또는 구속을 당한 자의 가족등 법률이 정하는 자에게는 그 이유와 일시·장소가 지체없이 통지되어야 한다.
                대한민국의 주권은 국민에게 있고, 모든 권력은 국민으로부터 나온다.국무회의는 정부의 권한에 속하는 중요한 정책을 심의한다.전직대통령의 신분과 예우에 관하여는 법률로 정한다.)", 
            result);
        }

        SUBCASE("Full Composite - Random String")
        {
            std::mt19937_64 rng{ 20000628 };
            std::uniform_int_distribution<int> dist{ L'가', L'힣' };

            constexpr int length = 50;
            constexpr int times = 5;

            for (int i = 0; i < times; i++)
            {
                std::wstring text;
                text.reserve(length);

                for (int j = 0; j < length; j++)
                {
                    text += static_cast<wchar_t>(dist(rng));
                }

                setup_imm_simulator();
                result.clear();
                imm_simulator_test(text, result);
                teardown_imm_simulator();
            }
        }

        SUBCASE("Mixed")
        {
            // Generated random alphabets with
            // `''.join([chr(n) if n <= ord('z') else "QWERTOP"[n - 1 - ord('z')] for n in [random.randint(ord('a'), ord('z') + 7) for j in range(100)]])`
            // Then used https://www.theyt.net/wiki/%ED%95%9C%EC%98%81%ED%83%80%EB%B3%80%ED%99%98%EA%B8%B0 to convert it to Hangeul.
            imm_simulator_test(L"ㅔㅑㅖ빼ㅔㅒㅓㅐㅉㅈㅁ쪱욯ㅊ두레ㅐㅓ쌰ㅒ켜ㅠㅋ히캍ㅎㅆ쭈햬쨮ㄹㅈㄷ쥬ㅒㅏㅐㅒㄸㅋ펴네ㅡㅛㅖㄱㅉㄸㅋㄹㄹ따뾰챩ㅃㅊㅂ뻬"
                               L"ㅜㅠ뿄ㅎㄲ졩ㅇ멭모ㅖㅠㅌㄸ햬ㅕㅓ츤ㅊㅆ괴ㅏㅁㅌ키ㅏ펻ㄷㅅㄲ쟹유력묘ㅖㅐㅜㅡ쨔ㅔ꺰츠ㅠㅋ뜨셰ㅕ럮내ㅔㅑㅠㅠㅈ치우븡혀ㅉ버ㅐㅓㅔ"
                               L"씨ㅔㅣㅗㅉㅆㅈ큐ㅒㅡ뷰서ㅔ튼암ㅃㅈㅌㅋ카ㅖㅛㅡ두ㅠㅎ주캐ㅠㅗㅓㅌ카ㅏㅋㅅㅍ쎴ㅌ챠ㅠㅎ훼ㅉㅁㄹㄸㅌㅇㄱㄷㅅㅉ찝꾜ㅉㄱ데뀨ㅠㅠ줴"
                               L"ㅔ맇ㅅ뇨뗘ㅖㅉㄱ하ㅔㅊㅃ쯔ㅐㅣㅠㅡ턍ㅋㄲㄴㅍㅇ떠ㅔㄸ사쎠뜢핫쨔ㅠㅣㅆㅂㄱㅈㅆㄹㄷ츠ㅓㄲㅋㅅㅋ됴ㅓㅌㅋㅊㄹㄸ넌렄ㄸㅇㅇㄴㅋ뗘펴"
                               L"ㅃ스ㅉ탂휘ㄸㅅ료ㅜ쟤ㅏㅅㅈ기ㅡ쿠ㅗㅕㄴ떠ㅛ껴ㅜㄱㅈ힢떠ㅖ롔써노외티ㅛ쁘뵤ㅛㅑㅃ뿌쁔ㅅ씹ㅈㅃㄲㅊ섁ㅉㅃ쩟쿠ㅡㅐㅔㅈ사ㅓㅔㅅ쟤ㅓ"
                               L"ㄲㄸ써ㅑㅑㅓㅏㅓㅔ럐ㅜㅏㅠㅜㅇ끼켸ㅓㅋㅍ헟ㅌ픈퍄ㅔㅇㅃㅉㄹ빨ㅆㅉㅌ소ㅓㅔㅠㅣㅁㄷ헫ㅈ뷰효ㅒㅒ우ㅒㅕㅖㅛㅛ키ㅛ드ㅏㄴ띠썤ㄴ도켸"
                               L"꽈ㅐㄸㅎ제ㅃㅇㅎㄱㄱㅇㄱㅌ규ㄸㅂ넇얘즈ㅃ나뀨녜ㅖ덎ㄱ캪ㄴㄱㅇㅋ븞ㄹㅃㅃㄴㅌㅁㅁㄸ끅ㅈㅋㅎㄴ쏍ㅉㅆㅁㅆㅁㅍㅉ갸ㅛ께죠ㅠㅛㅕㅗㅉ"
                               L"끼ㅗㅂㅆ퍠ㅛ볘ㅠㅑㅆㅈㅂㅎㅁ볘ㅑㅛㅡㅓㅍ섙ㅈㅃㅁㅎㄱ쎙ㅎㅆ푸ㅏㄹ쌪프시ㅕㅔㅔ쟈섓ㅍ냿ㄴㅌㅊㅎㅇㄱㅇㅊ쨔ㅓㅑㅜ뗴에ㅐㅎ요쎟ㅆㄸ"
                               L"얊ㅆ조캬쥬ㅗㅛㄱㅉㄴㄷ배라ㅒㅋㅌ젘ㅎㅋㄾ녜ㅔㅅㅂㅁㅉ께쑈ㅣㄴ녜ㅗㅊ네ㅗㅕㅕㅑㅇㅁㄲㄹㄹ례ㅡㅠㅎㄹ걮뀨ㅏㅜㅗ봏ㅎㅆㅁㅌㄹㅆㅇ읗"
                               L"ㅊㅁㅊ댸ㅓㅒㄷㅎ껴ㅛㅓㅈㅇ므ㅗㄷㄹ샤ㅓㅛ렾크추욤ㅃ레쳐어테ㅒ햬쟈코횹ㅊㅋ개ㅛ빔ㅍㄴㄱㅈ께ㅓㅅㅊㅃ럐ㅜ교ㅡㅡ뉴ㅕㅑㅁㄿㅆ례ㅜㄴ"
                               L"슈햬ㅒㅑㅏ추ㅒㄴ녜ㄸㅋ뉴ㅔㅐㄸㄱ떄티ㅐㅜㅊ젞앸썌ㅕ헇ㅋㅌㅈ려썌찡ㅌ째ㅐㅑㅍㅊㄽㄼ류ㅑㅒㅕㅔㅁ크두ㅐㅊㅇ빼ㅐㅛㄱㅁㅍㄸㅍㅆㅌㅃ"
                               L"ㄱㄷ드ㅗㅒㄲㅁ댸ㅕㅡ셔누ㅕㅇㅃ데ㅐㅅㄲㅋㅉㄱ", result);
        }

        std::erase_if(input_listeners, [](const std::pair<std::string, InputListener>& pair) { return pair.first == "test"; });
        teardown_imm_simulator();
    }
}
