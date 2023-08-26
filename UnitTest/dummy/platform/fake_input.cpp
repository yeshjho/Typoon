#include "../../Typoon/low_level/fake_input.h"

#include "../../util/text_editor_simulator.h"


// The values are from the unassigned block of the Unicode
const wchar_t FakeInput::BACKSPACE_KEY = 0x0870;
const wchar_t FakeInput::TOGGLE_HANGEUL_KEY = 0x0871;
const wchar_t FakeInput::LEFT_ARROW_KEY = 0x0872;
const wchar_t FakeInput::ENTER_KEY = 0x0873;


void send_fake_inputs(const std::vector<FakeInput>& inputs, [[maybe_unused]] bool isCapsLockOn)
{
    text_editor_simulator.Type(inputs);
}

