#include "text_editor_simulator.h"

#include "../../Typoon/imm/imm_simulator.h"
#include "../../Typoon/utils/string.h"


TextEditorSimulator::TextEditorSimulator()
{
    const auto lambdaListener = [this](const InputMessage(&messages)[MAX_INPUT_COUNT], int length, bool = false) { onInput(messages, length); };

    input_listeners.emplace_back("text editor simulator", lambdaListener);
}


void TextEditorSimulator::Type(const std::vector<FakeInput>& inputs)
{
    for (const auto& [type, letter] : inputs)
    {
        switch (type)
        {
        case FakeInput::EType::LETTER:
        {
            mText.insert(mText.begin() + mCursorPos, letter);
            mCursorPos++;
            break;
        }

        case FakeInput::EType::KEY:
        {
            if (letter == FakeInput::BACKSPACE_KEY)
            {
                imm_simulator.AddLetter('\b');
            }
            else if (letter == FakeInput::TOGGLE_HANGEUL_KEY)
            {
                imm_simulator.EmitAndClearCurrentComposite();
            }
            else if (letter == FakeInput::LEFT_ARROW_KEY)
            {
                if (mCursorPos > 0)
                {
                    mCursorPos--;
                }
            }
            else
            {
                std::unreachable();
            }

            break;
        }

        case FakeInput::EType::LETTER_AS_KEY:
            break;

        default:
            std::unreachable();
        }
    }

}


void TextEditorSimulator::Reset()
{
    mText.clear();
    mCursorPos = 0;
    imm_simulator.ClearComposition();
}


std::wstring TextEditorSimulator::GetText() const
{
    if (const wchar_t letterBeingComposed = imm_simulator.ComposeLetter();
        letterBeingComposed == 0)
    {
        return mText;
    }
    else
    {
        std::wstring text = mText;
        text.push_back(imm_simulator.ComposeLetter());
        return text;
    }
}


void TextEditorSimulator::onInput(const InputMessage(&messages)[MAX_INPUT_COUNT], int length)
{
    for (int i = 0; i < length; i++)
    {
        const auto [letter, isBeingComposed] = messages[i];
        if (!isBeingComposed)
        {
            if (letter == '\b')
            {
                if (mCursorPos > 0)
                {
                    mText.erase(mText.begin() + mCursorPos - 1);
                    mCursorPos--;
                }
            }
            else
            {
                mText.insert(mText.begin() + mCursorPos, letter);
                mCursorPos++;
            }
        }
    }
}
