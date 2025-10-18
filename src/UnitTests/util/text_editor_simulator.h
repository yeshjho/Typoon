#pragma once
#include "../../Typoon/imm/imm_simulator.h"
#include "../../Typoon/low_level/fake_input.h"


struct TextState
{
    std::wstring_view text;
    bool isLetterAtCursorInComposition = false;
    unsigned int cursorPos = std::numeric_limits<unsigned int>::max();

    std::wstring_view cursorPlaceholder = L"|_|";

    std::pair<unsigned int/* cursor pos */, unsigned int/* end pos */> RemoveCursorPos(std::wstring& workOn) const;
};


class TextEditorSimulator
{
public:
    TextEditorSimulator();

    void Type(wchar_t letter);
    void Type(const std::vector<FakeInput>& inputs);
    void Reset();

    [[nodiscard]] std::wstring GetText() const;
    [[nodiscard]] unsigned int GetCursorPos() const { return mCursorPos; }
    [[nodiscard]] bool IsLetterAtCursorInComposition() const;

    [[nodiscard]] bool operator==(const TextState& textState) const;

private:
    void onInput(const InputMessage(&messages)[MAX_INPUT_COUNT], int length);


private:
    std::wstring mText;
    unsigned int mCursorPos = 0;
    ImmSimulator mImmSimulator;
};


inline TextEditorSimulator text_editor_simulator;
