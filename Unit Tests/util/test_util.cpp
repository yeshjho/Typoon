#include "test_util.h"

#include "../../Typoon/imm/imm_simulator.h"
#include "../../Typoon/match/trigger_tree.h"
#include "config.h"
#include "text_editor_simulator.h"


void simulate_type(std::wstring_view text)
{
    for (const wchar_t letter : text)
    {
        // Order matters! If you think about it, when typing a letter in a text editor,
        // The letter will actually be typed first, and then Typoon will handle it.
        text_editor_simulator.Type(letter);
        imm_simulator.AddLetter(letter);
    }
}


void BasicMatchTest::SetUp(std::string_view matchesString)
{
    setup_trigger_tree("");
    reconstruct_trigger_tree(matchesString);
    setup_imm_simulator();
    set_config(default_config);
    text_editor_simulator.Reset();
    wait_for_trigger_tree_construction();
}


void BasicMatchTest::TearDown()
{
    teardown_trigger_tree();
    teardown_imm_simulator();
}
