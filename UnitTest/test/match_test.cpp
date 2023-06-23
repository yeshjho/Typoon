#include <doctest.h>

#include "../../Typoon/match/trigger_tree.h"
#include "../util/test_util.h"
#include "../util/config.h"
#include "../util/text_editor_simulator.h"


void start_test_case(const Config& config = default_config)
{
    setup_trigger_tree("");
    setup_imm_simulator();
    set_config(config);
    text_editor_simulator.Reset();
}

void end_test_case()
{
    teardown_trigger_tree();
    teardown_imm_simulator();
}


TEST_SUITE("Match")
{
    TEST_CASE("Erroneous Matches")
    {
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
    }

    TEST_CASE("Basics")
    {
        start_test_case();

        SUBCASE("Ascii Only")
        {
            reconstruct_trigger_tree(R"({
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

            CHECK(text_editor_simulator == TextState{ L"accommodate the necessary calendar with acknowledgment.\n"
                                                      "1, 2, 3, 4, 5, 6, 7, 8, 9, 10!@#$%^&*()" });
        }

        SUBCASE("Hangeul Only")
        {
            reconstruct_trigger_tree(R"({
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

            CHECK(text_editor_simulator == TextState{ L"예? 알겠습니다 감사합니다. 덩굴이 됐습니다." });
        }

        SUBCASE("Ascii & Hangeul Mixed")
        {
        }

        SUBCASE("Whitespaces")
        {
            reconstruct_trigger_tree(R"({
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

            CHECK(text_editor_simulator == TextState{ L"a    ba\nb가나" });
        }

        SUBCASE("Multiple Triggers")
        {
            reconstruct_trigger_tree(R"({
                matches: [
                    {
                        triggers: ['asc ii!', '한글', 'mixed혼합!'],
                        replace: '대체 text'
                    }
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L"asc ii! 한글\nmixed혼합!");

            CHECK(text_editor_simulator == TextState{ L"대체 text 대체 text\n대체 text" });
        }

        SUBCASE("Cursor Position")
        {
            reconstruct_trigger_tree(R"({
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
            CHECK(text_editor_simulator == TextState{ L"static_cast<int>(floatValue|_|)" });

            teardown_imm_simulator();
            setup_imm_simulator();
            text_editor_simulator.Reset();
            simulate_type(L"useless");
            CHECK(text_editor_simulator == TextState{ L"useless" });
        }

        end_test_case();
    }

    TEST_CASE("Options - case_sensitive")
    {
        
    }

    TEST_CASE("Options - word")
    {
        
    }

    TEST_CASE("Options - full_composite")
    {
        
    }

    TEST_CASE("Options - propagate_case")
    {
        
    }

    TEST_CASE("Options - uppercase_style")
    {
        
    }

    TEST_CASE("Options - keep_composite")
    {
        start_test_case();

        SUBCASE("Chaining")
        {
            reconstruct_trigger_tree(R"({
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

            CHECK(text_editor_simulator == TextState{ L"가a|_|단", true });
        }

        end_test_case();
    }
}


TEST_SUITE("Config")
{
    TEST_CASE("max_backspace_count")
    {
        
    }

    TEST_CASE("cursor_placeholder")
    {
        
    }
}
