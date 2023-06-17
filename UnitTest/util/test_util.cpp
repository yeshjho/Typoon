#include "test_util.h"

#include "text_editor_simulator.h"


void simulate_type(std::wstring_view text)
{
    for (wchar_t letter : text)
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
