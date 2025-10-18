#include "text_editor_simulator.h"

#include "../../Typoon/utils/string.h"


std::pair<unsigned, unsigned> TextState::RemoveCursorPos(std::wstring& workOn) const
{
    auto endPos = static_cast<unsigned int>(text.size());

    unsigned int newCursorPos = cursorPos;
    if (newCursorPos == std::numeric_limits<unsigned int>::max())
    {
        if (const size_t cursorIndex = text.find(cursorPlaceholder);
            cursorIndex != std::wstring::npos)
        {
            newCursorPos = static_cast<unsigned int>(cursorIndex);

            const auto placeholderSize = cursorPlaceholder.size();
            workOn.erase(cursorIndex, placeholderSize);
            endPos -= static_cast<unsigned int>(placeholderSize);
        }
        else
        {
            newCursorPos = endPos;
        }
    }

    return { newCursorPos, endPos };
}


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
                mImmSimulator.EmitAndClearCurrentComposite();
                if (mCursorPos > 0)
                {
                    mCursorPos--;
                }
            }
            else if (letter == FakeInput::ENTER_KEY)
            {
                mImmSimulator.EmitAndClearCurrentComposite();
                mImmSimulator.AddLetter('\n');
            }
            else
            {
                std::unreachable();
            }

            break;
        }

        case FakeInput::EType::LETTER_AS_KEY:
            // Only used for typing hangeul letters currently.
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
    if (const wchar_t letterBeingComposed = mImmSimulator.GetComposition().ComposeLetter();
        letterBeingComposed == 0)
    {
        return mText;
    }
    else
    {
        std::wstring text = mText;
        text.insert(text.begin() + mCursorPos, letterBeingComposed);
        return text;
    }
}


bool TextEditorSimulator::IsLetterAtCursorInComposition() const
{
    return mImmSimulator.GetComposition().ComposeLetter() != 0;
}


bool TextEditorSimulator::operator==(const TextState& textState) const
{
    if (textState.isLetterAtCursorInComposition != IsLetterAtCursorInComposition())
    {
        return false;
    }

    std::wstring textToCompare{ textState.text };

    const auto [cursorPos, endPos] = textState.RemoveCursorPos(textToCompare);

    if (cursorPos != mCursorPos)
    {
        return false;
    }

    return textToCompare == GetText();
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
