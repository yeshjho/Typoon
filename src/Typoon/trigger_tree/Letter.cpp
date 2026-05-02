#include "Letter.h"

#include <cwctype>
#include <tuple>

#include "util/Assert.h"
#include "util/String.h"


namespace typoon::core
{
    Letter::Letter(const wchar_t letter, const bool isCaseSensitive, const bool doNeedFullComposite)
        : mIsCaseSensitive(isCaseSensitive && util::is_latin_alphabet(letter))
        , mLetterLowered(std::towlower(letter))
        , mLetter(mIsCaseSensitive ? letter : mLetterLowered)
        , mDoNeedFullComposite(doNeedFullComposite)
        , mIsSpecial(letter == NON_WORD_LETTER)
    {
    }

    bool Letter::operator<(const Letter& other) const
    {
        // 특수 문자 (같은 특수문자끼리는 값순으로 정렬) <
        // A(대소문자 구별) < a(대소문자 구별 안 함) < a(대소문자 구별) <
        // B(대소문자 구별) < b(대소문자 구별 안 함) < b(대소문자 구별) ...

        return
            std::make_tuple(!mIsSpecial, mLetterLowered, mLetter, mIsCaseSensitive)
            <
            std::make_tuple(!other.mIsSpecial, other.mLetterLowered, other.mLetter, other.mIsCaseSensitive);
    }

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

    bool Letter::operator<(const wchar_t ch) const
    {
        ASSERT(Letter{ ch }.mIsSpecial == false);

        // 특수 문자인 경우 항상 true 반환 (특수 문자는 == 체크에서만 사용)
        //            A와 equivalent
        // ┌----------------------------------┐
        // A(대소문자 구별) < a(대소문자 구별 안 함) < a(대소문자 구별)
        //                  └----------------------------------┘
        //                              a와 equivalent

        if (mIsSpecial)
        {
            return true;
        }

        if (const std::strong_ordering letterLoweredComp = mLetterLowered <=> std::towlower(ch);
            letterLoweredComp != std::strong_ordering::equal)
        {
            return letterLoweredComp == std::strong_ordering::less;
        }

        if (mIsCaseSensitive)
        {
            return mLetter < ch;
        }
        else
        {
            return false;
        }
    }

    bool operator<(const wchar_t ch, const Letter& letter)
    {
        ASSERT(Letter{ ch }.mIsSpecial == false);

        if (letter.mIsSpecial)
        {
            return false;
        }

        if (const std::strong_ordering letterLoweredComp = std::towlower(ch) <=> letter.mLetterLowered;
            letterLoweredComp != std::strong_ordering::equal)
        {
            return letterLoweredComp == std::strong_ordering::less;
        }

        if (letter.mIsCaseSensitive)
        {
            return ch < letter.mLetter;
        }
        else
        {
            return false;
        }
    }
}
