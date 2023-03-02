#pragma once
#include <vector>


struct FakeInput
{
    enum class EType
    {
        LETTER,
        BACKSPACE,
    };

    EType type;
    wchar_t letter;
};


void send_fake_inputs(const std::vector<FakeInput>& inputs);
