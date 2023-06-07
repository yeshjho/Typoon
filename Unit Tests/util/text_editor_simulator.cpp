#include "text_editor_simulator.h"

#include "../../Typoon/utils/string.h"


TextEditorSimulator::TextEditorSimulator()
{
    mImmSimulator.RedirectInputMulticast([this](const InputMessage(&messages)[MAX_INPUT_COUNT], int length, bool = false) { onInput(messages, length); });
}


void TextEditorSimulator::Type(wchar_t letter)
{
    mImmSimulator.AddLetter(letter);
}


void TextEditorSimulator::Type(const std::vector<FakeInput>& inputs)
{
    for (const auto& [type, letter] : inputs)
    {
        switch (type)
        {
        case FakeInput::EType::LETTER:
        {
            mImmSimulator.EmitAndClearCurrentComposite();
            // This case bypasses the imm irl, too
            mText.insert(mText.begin() + mCursorPos, letter);
            mCursorPos++;
            break;
        }

        case FakeInput::EType::KEY:
        {
            if (letter == FakeInput::BACKSPACE_KEY)
            {
                mImmSimulator.AddLetter('\b');
            }
            else if (letter == FakeInput::TOGGLE_HANGEUL_KEY)
            {
                mImmSimulator.EmitAndClearCurrentComposite();
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
            for (const wchar_t hangeul : alphabet_to_hangeul({ &letter, 1 }))
            {
                mImmSimulator.AddLetter(hangeul);
            }
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
    mImmSimulator.ClearComposition();
}


std::wstring TextEditorSimulator::GetText() const
{
    if (const wchar_t letterBeingComposed = mImmSimulator.ComposeLetter();
        letterBeingComposed == 0)
    {
        return mText;
    }
    else
    {
        std::wstring text = mText;
        text.push_back(mImmSimulator.ComposeLetter());
        return text;
    }
}


// This will be called by the internal imm simulator
void TextEditorSimulator::onInput(const InputMessage(&messages)[MAX_INPUT_COUNT], int length)
{
    for (int i = 0; i < length; i++)
    {
        const auto [letter, isBeingComposed] = messages[i];
        if (isBeingComposed)
        {
            continue;
        }

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
