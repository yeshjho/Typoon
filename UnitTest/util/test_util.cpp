#include "test_util.h"

#include "../../Typoon/utils/string.h"
#include "../../Typoon/match/trigger_tree.h"
#include "text_editor_simulator.h"


void simulate_type(std::wstring_view text)
{
    const std::wstring toType = normalize_hangeul(text);
    for (wchar_t letter : toType)
    {
        if (letter == FakeInput::TOGGLE_HANGEUL_KEY)
        {
            imm_simulator.EmitAndClearCurrentComposite();
        }
        else if (letter == FakeInput::LEFT_ARROW_KEY)
        {
            throw std::runtime_error{ "Simulating the left arrow key is not supported." };
        }
        else
        {
            if (letter == FakeInput::BACKSPACE_KEY)
            {
                letter = '\b';
            }

            // Order matters! If you think about it, when typing a letter in a text editor,
            // The letter will actually be typed first, and then Typoon will handle it.
            text_editor_simulator.Type(letter);
            imm_simulator.AddLetter(letter);
        }
    }
}


void reconstruct_trigger_tree_with_u8string(std::u8string_view text)
{
    reconstruct_trigger_tree(std::string_view{ reinterpret_cast<const char*>(&text.front()), text.size() });
}


void check_text_editor_simulator(const TextState& textState)
{
    CHECK(text_editor_simulator == textState);
}


void check_normalization(std::wstring_view original, std::wstring_view normalized)
{
    CHECK(normalize_hangeul(original) == normalized);
}
