#include <doctest.h>

#include "../../Typoon/match/trigger_tree.h"
#include "../util/test_util.h"
#include "../util/config.h"
#include "../util/text_editor_simulator.h"


TEST_CASE("TestTest")
{
    setup_trigger_tree("");
    setup_imm_simulator();
    set_config(default_config);
    text_editor_simulator.Reset();

    SUBCASE("sub")
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

    SUBCASE("sub2")
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

    teardown_trigger_tree();
    teardown_imm_simulator();
}
