#include "switch_hangeul_alphabet.h"

#include <algorithm>

#include "../input_multicast/input_multicast.h"
#include "../utils/config.h"

// TODO: listen for config change and resize - make a config change event
std::wstring stroke;
wchar_t letter_being_composed = 0;


void on_input(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents)
{
    if (clearAllAgents)
    {
        std::ranges::fill(stroke, L'\0');
    }

    for (int i = 0; i < length; i++)
    {
        const auto [inputLetter, isBeingComposed] = inputs[i];

        if (isBeingComposed)
        {
            letter_being_composed = inputLetter;
        }
        else
        {
            std::ranges::shift_left(stroke, 1);
            stroke.back() = inputLetter;
        }
    }
}


void setup_switcher()
{
    input_listeners.emplace_back("switch_hangeul_alphabet", on_input);
    stroke.resize(get_config().strokeBuffer, 0);
}


void teardown_switcher()
{
    std::erase_if(input_listeners, [](const std::pair<std::string, InputListener>& pair) { return pair.first == "switch_hangeul_alphabet"; });
}


void switch_hangeul_alphabet()
{
    
}
