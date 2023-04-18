#pragma once
#include <functional>


struct InputMessage
{
    wchar_t letter = 0;
    bool isBeingComposed = false;
};

constexpr int MAX_INPUT_COUNT = 2;

void multicast_input(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents = false);
void register_input_listener(std::function<void(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents)> listener);
