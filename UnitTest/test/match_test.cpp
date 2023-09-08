#include <doctest.h>

#include "../../Typoon/match/trigger_tree.h"
#include "../util/test_util.h"


TEST_SUITE("Match")
{
    TEST_CASE("Erroneous Matches")
    {
        start_match_test_case();

        SUBCASE("Triggers")
        {
            // No Trigger
            // Empty Trigger
        }

        SUBCASE("Replace")
        {
            // No Replace
            // Empty Replace
        }

        SUBCASE("Hiding Triggers")
        {

        }

        SUBCASE("Infinite Triggers - Ignore keep_composite")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: '가',
                        replace: '나가',
                        keep_composite: true
                    },
                    {
                        trigger: 'ㄷ',
                        replace: 'ㄷ',
                        keep_composite: true
                    }
               ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"가 가가가ㄷㄷㅏ");
            check_text_editor_simulator({ L"나가 나가나가나가ㄷㄷ|_|ㅏ", true });
        }

        end_match_test_case();
    }

    TEST_CASE("Basics")
    {
        start_match_test_case();

        SUBCASE("Ascii Only")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'acommodate',
                        replace: 'accommodate',
                    },
                    {
                        trigger: 'acknowledgement',
                        replace: 'acknowledgment',
                    },
                    {
                        trigger: 'calender',
                        replace: 'calendar'
                    },
                    {
                        trigger: 'neccessary',
                        replace: 'necessary',
                    },
                    {
                        trigger: ':1to10:',
                        replace: '1, 2, 3, 4, 5, 6, 7, 8, 9, 10'
                    },
                    {
                        trigger: '!to)',
                        replace: '!@#$%^&*()'
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"acommodate the neccessary calender with acknowledgement.\n"
                          ":1to10:!to)");

            check_text_editor_simulator({ L"accommodate the necessary calendar with acknowledgment.\n"
                                                      "1, 2, 3, 4, 5, 6, 7, 8, 9, 10!@#$%^&*()" });
        }

        SUBCASE("Hangeul Only")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: '됬',
                        replace: '됐',
                    },
                    {
                        trigger: '덩쿨',
                        replace: '덩굴',
                    },
                    {
                        trigger: 'ㄳ',
                        replace: '감사합니다'
                    },
                    {
                        trigger: 'ㅖ?',
                        replace: "예?"
                    },
                    {
                        trigger: ';ㅇㅇ',
                        replace: '알겠습니다'
                    },
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"ㅖ? ;ㅇㅇ ㄳ. 덩쿨이 됬습니다.");

            check_text_editor_simulator({ L"예? 알겠습니다 감사합니다. 덩굴이 됐습니다." });
        }

        SUBCASE("Ascii & Hangeul Mixed")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'aㄱb나!',
                        replace: '혼합mixed'
                    },
                    {
                        trigger: '가aㄴbㄳ',
                        replace: 'mixed혼합'
                    },
                    {
                        trigger: '혼합mixed',
                        replace: 'mixed혼합'
                    },
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"aㄱb나! 가aㄴbㄳ\n혼합mixed");

            check_text_editor_simulator({ L"혼합mixed mixed혼합\nmixed혼합" });
        }

        SUBCASE("Whitespaces")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'ab',
                        replace: 'a
b',
                    },
                    {
                        trigger: 'a	b',  // tab
                        replace: 'a    b',  // 4 spaces
                    },
                    {
                        trigger: '가
나',
                        replace: '가나'
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"a	bab가\n나");

            check_text_editor_simulator({ L"a    ba\nb가나" });
        }

        SUBCASE("Multiple Triggers")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        triggers: ['asc ii!', '한글', 'mixed혼합!'],
                        replace: '대체 text'
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"asc ii! 한글\nmixed혼합!");

            check_text_editor_simulator({ L"대체 text 대체 text\n대체 text" });
        }

        SUBCASE("Cursor Position")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: ';sti',
                        replace: 'static_cast<int>(|_|)'
                    },
                    {
                        trigger: 'useless',
                        replace: 'useless|_|'
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L";stifloatValue");
            check_text_editor_simulator({ L"static_cast<int>(floatValue|_|)" });

            teardown_imm_simulator();
            setup_imm_simulator();
            text_editor_simulator.Reset();
            simulate_type(L"useless");
            check_text_editor_simulator({ L"useless" });
        }

        end_match_test_case();
    }

    TEST_CASE("Options - case_sensitive")
    {
        start_match_test_case();

        SUBCASE("Case Sensitive")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'lower',
                        replace: 'triggered',
                        case_sensitive: true,
                    },
                    {
                        trigger: 'UPPER',
                        replace: 'TRIGGERED',
                        case_sensitive: true,
                    },
                    {
                        trigger: 'MiXeD',
                        replace: 'tRiGgErEd',
                        case_sensitive: true,
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"lower LOWER lOwER upper UPPER upPeR mixed MIXED MiXeD");

            check_text_editor_simulator({ L"triggered LOWER lOwER upper TRIGGERED upPeR mixed MIXED tRiGgErEd" });
        }

        SUBCASE("Case Insensitive")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'lower',
                        replace: 'triggered',
                    },
                    {
                        trigger: 'UPPER',
                        replace: 'TRIGGERED',
                        case_sensitive: false,
                    },
                    {
                        trigger: 'MiXeD',
                        replace: 'tRiGgErEd',
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"lower LOWER lOwER upper UPPER upPeR mixed MIXED MiXeD");

            check_text_editor_simulator({ L"triggered triggered triggered TRIGGERED TRIGGERED TRIGGERED tRiGgErEd tRiGgErEd tRiGgErEd" });
        }

        end_match_test_case();
    }

    TEST_CASE("Options - word")
    {
        start_match_test_case();

        SUBCASE("Word")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'apple',
                        replace: 'banana',
                        word: true,
                    },
                    {
                        trigger: '가나',
                        replace: '다라',
                        word: true,
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"apple, apples apple\n가나하 가나. 가나ㅏ ");

            check_text_editor_simulator({ L"banana, apples banana\n가나하 다라. 가나ㅏ " });
        }

        SUBCASE("Non Word")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'apple',
                        replace: 'banana',
                    },
                    {
                        trigger: '가나',
                        replace: '다라',
                        word: false,
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"apple, apples apple\n가나하 가나. 가나ㅏ ");

            check_text_editor_simulator({ L"banana, bananas banana\n다라하 다라. 다라ㅏ " });
        }

        end_match_test_case();
    }

    TEST_CASE("Options - full_composite")
    {
        start_match_test_case();

        SUBCASE("Full Composite")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: '가나',
                        replace: '다라',
                        full_composite: true
                    },
                    {
                        trigger: 'ㄳ',
                        replace: '감사',
                        full_composite: true
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"가나 가나카 가난한 가나, ㄱ사 ㄳ. ㄳ ");

            check_text_editor_simulator({ L"다라 다라카 가난한 다라, ㄱ사 감사. 감사 " });
        }

        SUBCASE("Non Full Composite")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: '가나',
                        replace: '다라',
                        full_composite: false
                    },
                    {
                        trigger: 'ㄳ',
                        replace: '감사',
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"가나 가나카 가난한 가나, ㄱ사 ㄳ. ㄳ ");

            check_text_editor_simulator({ L"다라 다라카 다라ㄴ한 다라, 감사ㅏ 감사. 감사 " });
        }

        end_match_test_case();
    }

    TEST_CASE("Options - propagate_case")
    {
        start_match_test_case();

        SUBCASE("Propagate Case")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'apple',
                        replace: 'banana',
                        propagate_case: true
                    },
                    {
                        trigger: ';car',
                        replace: '1d2o3g',
                        propagate_case: true
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"apple Apple APplE aPplE APPLE ;car ;Car ;CAr ;cAr ;CAR");

            check_text_editor_simulator({ L"banana Banana Banana banana BANANA 1d2o3g 1D2o3g 1D2o3g 1d2o3g 1D2O3G" });
        }

        SUBCASE("Don't Propagate Case")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'apple',
                        replace: 'banana',
                        propagate_case: false,
                    },
                    {
                        trigger: ';car',
                        replace: '1d2o3g',
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"apple Apple APplE aPplE APPLE ;car ;Car ;CAr ;cAr ;CAR");

            check_text_editor_simulator({ L"banana banana banana banana banana 1d2o3g 1d2o3g 1d2o3g 1d2o3g 1d2o3g" });
        }

        end_match_test_case();
    }

    TEST_CASE("Options - uppercase_style")
    {
        start_match_test_case();

        SUBCASE("Default (First Letter)")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'apple',
                        replace: 'banana butterfly',
                        propagate_case: true,
                        uppercase_style: 'first_letter',
                    },
                    {
                        trigger: ';car',
                        replace: '1d2o3g dr!1l',
                        propagate_case: true,
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"apple Apple APplE aPplE APPLE ;car ;Car ;CAr ;cAr ;CAR");

            check_text_editor_simulator({ L"banana butterfly Banana butterfly Banana butterfly banana butterfly BANANA BUTTERFLY "
                                                      "1d2o3g dr!1l 1D2o3g dr!1l 1D2o3g dr!1l 1d2o3g dr!1l 1D2O3G DR!1L" });
        }

        SUBCASE("Word")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'apple',
                        replace: 'banana butterfly',
                        propagate_case: true,
                        uppercase_style: 'capitalize_words',
                    },
                    {
                        trigger: ';car',
                        replace: '1d2o3g dr!1l',
                        propagate_case: true,
                        uppercase_style: 'capitalize_words',
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"apple Apple APplE aPplE APPLE ;car ;Car ;CAr ;cAr ;CAR");

            check_text_editor_simulator({ L"banana butterfly Banana Butterfly Banana Butterfly banana butterfly BANANA BUTTERFLY "
                                                      "1d2o3g dr!1l 1D2o3g Dr!1L 1D2o3g Dr!1L 1d2o3g dr!1l 1D2O3G DR!1L" });
        }

        end_match_test_case();
    }

    TEST_CASE("Options - keep_composite")
    {
        start_match_test_case();

        SUBCASE("Keep Composite")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: '스빈다',
                        replace: '습니다',
                        keep_composite: true,
                    },
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"알겠스빈답");

            check_text_editor_simulator({ L"알겠습니|_|답", true });
        }

        SUBCASE("Don't Keep Composite")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: '스빈다',
                        replace: '습니다',
                    },
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"알겠스빈답");

            check_text_editor_simulator({ L"알겠습니다|_|ㅂ", true });
        }

        SUBCASE("Chaining")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: 'wwww',
                        replace: '가a나',
                        keep_composite: true,
                    },
                    {
                        trigger: '난',
                        replace: '단',
                        keep_composite: true,
                    },
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"wwwwㄴ");

            check_text_editor_simulator({ L"가a|_|단", true });
        }

        end_match_test_case();
    }

    TEST_CASE("Mixed Options")
    {
        // Writing a test for all possible combinations is simply too much work. (2^6 - 1(all off) - 6(only one on) = 57)
        // Therefore, only test for 2 combinations of each of them. (6C2 = 15)
        start_match_test_case();

        SUBCASE("")
        {
            
        }

        end_match_test_case();
    }
}


TEST_SUITE("Config")
{
    TEST_CASE("max_backspace_count")
    {
        Config config = default_config;

        SUBCASE("Default Amount (5)")
        {
            start_match_test_case(config);

            reconstruct_trigger_tree_with_u8string(u8R"({
                    matches: [
                        {
                            trigger: 'apple',
                            replace: 'banana',
                        },
                    ]
                })");
            wait_for_trigger_tree_construction();

            simulate_type(L"a[[;l\b\b\b\bpple\na[[;l33\b\b\b\b\b\bpple aooooo\b\b\b\b\bpple");

            check_text_editor_simulator({ L"banana\napple banana" });
        }

        SUBCASE("Lesser Amount")
        {
            config.maxBackspaceCount = 3;
            start_match_test_case(config);

            reconstruct_trigger_tree_with_u8string(u8R"({
                    matches: [
                        {
                            trigger: 'apple',
                            replace: 'banana',
                        },
                    ]
                })");
            wait_for_trigger_tree_construction();

            simulate_type(L"a[[;l\b\b\b\bpple\na[[;\b\b\bpple");

            check_text_editor_simulator({ L"apple\nbanana" });
        }

        SUBCASE("Unforgiving")
        {
            config.maxBackspaceCount = -123;
            start_match_test_case(config);

            reconstruct_trigger_tree_with_u8string(u8R"({
                    matches: [
                        {
                            trigger: 'apple',
                            replace: 'banana',
                        },
                    ]
                })");
            wait_for_trigger_tree_construction();

            simulate_type(L"app;\ble");

            check_text_editor_simulator({ L"apple" });
        }

        end_match_test_case();
    }

    TEST_CASE("cursor_placeholder")
    {
        Config config = default_config;
        config.cursorPlaceholder = L"$|$";

        start_match_test_case(config);

        SUBCASE("No More |_|")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: ';sti',
                        replace: 'static_cast<int>(|_|)'
                    },
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L";stifloatValue");
            check_text_editor_simulator({ .text = L"static_cast<int>(|_|)floatValue", .cursorPlaceholder = L"$|$" });
        }

        SUBCASE("Cursor Placeholder")
        {
            reconstruct_trigger_tree_with_u8string(u8R"({
                matches: [
                    {
                        trigger: ';sti',
                        replace: 'static_cast<int>($|$)'
                    },
                    {
                        trigger: 'useless',
                        replace: 'useless$|$'
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L";stifloatValue");
            check_text_editor_simulator({ .text = L"static_cast<int>(floatValue$|$)", .cursorPlaceholder = L"$|$" });

            teardown_imm_simulator();
            setup_imm_simulator();
            text_editor_simulator.Reset();
            simulate_type(L"useless");
            check_text_editor_simulator({ .text = L"useless", .cursorPlaceholder = L"$|$" });
        }

        end_match_test_case();
    }
}
