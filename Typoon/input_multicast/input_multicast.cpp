#include "input_multicast.h"


std::vector<std::function<void(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents)>> listeners;

void multicast_input(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents)
{
    for (auto& listener : listeners)
    {
        listener(inputs, length, clearAllAgents);
    }
}


void register_input_listener(std::function<void(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents)> listener)
{
    listeners.emplace_back(std::move(listener));
}
