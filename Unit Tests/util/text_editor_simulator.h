#pragma once
#include "../../Typoon/input_multicast/input_multicast.h"
#include "../../Typoon/low_level/fake_input.h"


class TextEditorSimulator
{
public:
    TextEditorSimulator();

    void Type(const std::vector<FakeInput>& inputs);
    void Reset();

    [[nodiscard]] std::wstring GetText() const;

private:
    void onInput(const InputMessage(&messages)[MAX_INPUT_COUNT], int length);


private:
    std::wstring mText;
    unsigned int mCursorPos = 0;
};


inline TextEditorSimulator text_editor_simulator;
