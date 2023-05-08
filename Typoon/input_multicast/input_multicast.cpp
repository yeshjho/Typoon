#include "input_multicast.h"

#include <ranges>


void multicast_input(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents)
{
    for (auto& listener : input_listeners | std::views::values)
    {
        listener(inputs, length, clearAllAgents);
    }
}
