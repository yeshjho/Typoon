#pragma once
#include "../../Typoon/imm/imm_simulator.h"
#include "../../Typoon/low_level/fake_input.h"


class TextEditorSimulator
{
public:
    TextEditorSimulator();

    void Type(wchar_t letter);
    void Type(const std::vector<FakeInput>& inputs);
    void Reset();

    [[nodiscard]] std::wstring GetText() const;

private:
    void onInput(const InputMessage(&messages)[MAX_INPUT_COUNT], int length);


private:
    std::wstring mText;
    unsigned int mCursorPos = 0;
    ImmSimulator mImmSimulator;
};


inline TextEditorSimulator text_editor_simulator;
