#include "Letter.h"

#include <cwctype>

#include "util/String.h"


namespace typoon::core
{

Letter::Letter(const wchar_t letter, const bool isCaseSensitive, const bool doNeedFullComposite)
    : mLetter(letter)
    , mLetterLowered(std::towlower(letter))
    , mIsCased(util::is_latin_alphabet(letter))
    , mIsCaseSensitive(isCaseSensitive && mIsCased)
    , mDoNeedFullComposite(doNeedFullComposite)
{}

bool Letter::operator==(const wchar_t ch) const
{
    if (mLetter == NON_WORD_LETTER)
    {
        return !std::iswalnum(ch);
    }

    if (mIsCaseSensitive || !util::is_latin_alphabet(ch))
    {
        return mLetter == ch;
    }

    return mLetterLowered == std::towlower(ch);
}

bool Letter::operator==(const Letter& other) const
{
    if (mLetter == other.mLetter)
    {
        // 대소문자 구별 여부도 equality에 영향을 미침
        return mIsCaseSensitive == other.mIsCaseSensitive;
    }

    if (mIsCaseSensitive || other.mIsCaseSensitive)
    {
        return false;
    }

    return mLetterLowered == other.mLetterLowered;
}

std::strong_ordering Letter::operator<=>(const Letter& other) const
{
    // 대소문자 구별하지 않고 같은 문자 -> 대소문자 구별 여부 -> 실제 문자 순으로 정렬

    if (mIsCased)
    {
        if (const std::strong_ordering comp = mLetterLowered <=> other.mLetterLowered;
            comp != std::strong_ordering::equal)
        {
            return comp;
        }

        // 대소문자 구별하는 경우가 앞에 오도록하기 위해 other <=> self
        if (const std::strong_ordering comp = other.mIsCaseSensitive <=> mIsCaseSensitive;
            comp != std::strong_ordering::equal)
        {
            return comp;
        }
    }

    return mLetter <=> other.mLetter;
}

}
