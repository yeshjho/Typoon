#include "input_multicast.h"


std::vector<std::function<void(InputMessage(&inputs)[MAX_INPUT_COUNT], int length)>> listeners;

void multicast_input(InputMessage(&inputs)[MAX_INPUT_COUNT], int length)
{
    for (auto& listener : listeners)
    {
        listener(inputs, length);
    }
}


void register_input_listener(std::function<void(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length)> listener)
{
    listeners.emplace_back(std::move(listener));
}
