#pragma once
#include <functional>
#include <string>


struct InputMessage
{
    wchar_t letter = 0;
    bool isBeingComposed = false;
};

constexpr int MAX_INPUT_COUNT = 2;

using InputListener = std::function<void(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents)>;
inline std::vector<std::pair<std::string, InputListener>> input_listeners;  // NOTE: This is not thread-safe, modify only in the main thread.


void multicast_input(const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents = false);
