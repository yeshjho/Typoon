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

        SUBCASE("Trigger - Ascii Only")
        {
            
        }

        SUBCASE("Trigger - Hangeul Only")
        {
            
        }

        SUBCASE("Trigger - Ascii & Hangeul Mixed")
        {
            
        }

        SUBCASE("Trigger - Whitespaces")
        {
            
        }

        SUBCASE("Replace - Ascii Only")
        {
            
        }

        SUBCASE("Replace - Hangeul Only")
        {
            
        }

        SUBCASE("Replace - Ascii & Hangeul Mixed")
        {
            
        }

        SUBCASE("Replace - Whitespaces")
        {
            
        }

        SUBCASE("Multiple Triggers")
        {
            
        }

        SUBCASE("Cursor Position")
        {
            reconstruct_trigger_tree(R"({
                matches: [
                    {
                        trigger: ';sti',
                        replace: 'static_cast<int>(|_|)'
                    },
                ]
            })");
            wait_for_trigger_tree_construction();

            simulate_type(L";stifloatValue");

            CHECK(text_editor_simulator == TextState{ L"static_cast<int>(floatValue|_|)" });
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

            CHECK(text_editor_simulator == TextState{ L"가a단", true });
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
