#include "test_util.h"

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
