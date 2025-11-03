#include "Composition.h"

#include "util/Assert.h"
#include "util/Hangeul.h"


namespace typoon::core
{

Composition::Composition(util::NullableCallback<wchar_t> compositeOutputCallback)
    : mCompositeOutputCallback(std::move(compositeOutputCallback))
{
}

void Composition::AddAlphabet(const wchar_t alphabet)
{
    const bool isConsonant = util::is_hangeul_consonant(alphabet);
    const bool isVowel = util::is_hangeul_vowel(alphabet);

    if (alphabet == '\b')
    {
        // 낱자가 제거됐다면 백스페이스를 현재 조합이 '흡수'한 것이므로 출력하지 않음.
        // 낱자가 제거되지 않았다면, 현재 조합이 비어있는 상태이므로 백스페이스를 그대로 출력함.
        if (!removeAlphabet())
        {
            mCompositeOutputCallback('\b');
        }
        return;
    }

    // 한글 낱자가 아니라면, 현재 조합을 끝낸 후 출력하고 들어온 낱자를 그대로 출력함
    if (!isConsonant && !isVowel)
    {
        composeAndResetAndRunCallback();
        mCompositeOutputCallback(alphabet);
        return;
    }

    if (isConsonant)
    {
        // 겹받침만 있는 글자도 존재할 수 있음.
        // 초성이 있는데 중성이 없고, 들어온 자음이 현재 초성과 합쳐질 수 있다면, 합쳐서 종성으로 만듦. 종성만 있는 글자가 될 것임.
        if (const bool hasInitial = mInitial != 0, hasMedial = mMedial[0] != 0;
            hasInitial && !hasMedial && CanCombineAlphabets(mInitial, alphabet))
        {
            mFinal[0] = mInitial;
            mFinal[1] = alphabet;
            mInitial = 0;
        }
        // 키 입력이 두 번 필요한 초성은 없으므로, 초성이 없는 상태라면 현재 조합을 끝낸 후 출력하고 들어온 낱자를 초성으로 설정함.
        // 초성이 없는데 현재 조합을 출력하는 이유는 종성만 있는 글자가 있을 수 있기 때문임. (ex - 'ㅏ' 상태에서 'ㄱ' 입력)
        else if (!hasInitial ||
        // 초성은 있는데 중성이 없는 경우도 이전 글자 조합이 끝났음을 알 수 있음. (ex - 'ㄱ' 상태에서 'ㄱ' 입력)
                 !hasMedial || 
        // 초성 및 중성이 있는 경우,
        // 종성이 가득 차 있거나 (ex - '갃' 상태에서 'ㄱ' 입력)
                 mFinal[1] != 0 ||
        // 종성이 되지 못하는 자음이거나 (ex - '가' 상태에서 'ㄸ' 입력)
                 !IsValidForFinal(alphabet) || 
        // 현재 종성이 들어온 자음과 합쳐질 수 없는 경우(ex - '간' 상태에서 'ㄱ' 입력)에도 이전 글자 조합이 끝났음을 알 수 있음.
                 !CanCombineAlphabets(mFinal[0], alphabet)
            )
        {
            composeAndResetAndRunCallback();
            mInitial = alphabet;
        }
        // 초성, 중성이 모두 있고, 현재 종성과 합쳐질 수 있음 (종성이 없는 경우 포함).
        else
        {
            mFinal[mFinal[0] == 0 ? 0 : 1] = alphabet;
        }
    }
    else  // isVowel
    {
        // 종성에 한 글자라도 있다면, 하나를 떼어내서 새 글자의 초성으로 사용. (ex - '각' 상태에서 'ㅑ' 입력 -> '가갸')
        // 현재 글자에 더 이상 낱자를 추가할 수 없어도 바로 글자 조합을 출력할 수 없는 이유이기도 함.
        if (mFinal[0] != 0)
        {
            wchar_t& consonantToDetach = mFinal[mFinal[1] == 0 ? 0 : 1];
            const wchar_t newInitial = consonantToDetach;
            consonantToDetach = 0;
            composeAndResetAndRunCallback();
            mInitial = newInitial;
            mMedial[0] = alphabet;
        }
        // 중성이 꽉 차 있거나 (ex - '과' 상태에서 'ㅏ' 입력) 현재 중성이 들어온 모음과 합쳐질 수 없는 경우 (ex - '구' 상태에서 'ㅏ' 입력),
        // 이전 글자 조합을 끝낸 후 출력하고 들어온 낱자를 중성으로 설정함 (초성 없이 중성만 있는 글자가 될 것임).
        else if (mMedial[1] != 0 || !CanCombineAlphabets(mMedial[0], alphabet))
        {
            composeAndResetAndRunCallback();
            mMedial[0] = alphabet;
        }
        // 현재 종성이 없고, 현재 중성과 합쳐질 수 있음 (중성이 없는 경우 포함).
        else
        {
            mMedial[mMedial[0] == 0 ? 0 : 1] = alphabet;
        }
    }
}

wchar_t Composition::ComposeLetter() const
{
    const wchar_t combinedMedial = CombineAlphabets(mMedial[0], mMedial[1]);
    const wchar_t combinedFinal = CombineAlphabets(mFinal[0], mFinal[1]);

    // 초성이나 중성이 없으면, 종성만 있는 글자임
    if (const bool hasInitial = mInitial != 0, hasMedial = combinedMedial != 0;
        !hasInitial && !hasMedial)
    {
        return combinedFinal;
    }
    // 초성이 없는 글자면, 중성만 있는 글자임
    else if (!hasInitial)
    {
        return combinedMedial;
    }
    // 중성이 없는 글자면, 초성만 있는 글자임
    else if (!hasMedial)
    {
        return mInitial;
    }

    constexpr wchar_t INVALID = 0xFFFF;

    // 자음 리스트 (유니코드 순):
    // ㄱ ㄲ ㄳ ㄴ ㄵ ㄶ ㄷ ㄸ ㄹ ㄺ ㄻ ㄼ ㄽ ㄾ ㄿ ㅀ ㅁ ㅂ ㅃ ㅄ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ (0x3131 ~ 0x314E)
    // 모든 자음이 초성/종성으로 쓰일 수 있지 않으므로 매핑해줘야 함.
    // 초, 중, 종성을 구분하는 0x1100 ~ 0x11FF를 사용하면 이 과정을 생략할 수 있으나 AddAlphabet()에서 변환해줘야 하므로 선택의 문제임.
    constexpr wchar_t INITIAL_MAP[] = {
        0, 1, INVALID, 2, INVALID, INVALID, 3, 4, 5, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, 6, 7, 8, INVALID, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
    };
    constexpr wchar_t FINAL_MAP[] = {
        0, 1, 2, 3, 4, 5, 6, INVALID, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, INVALID, 17, 18, 19, 20, 21, INVALID, 22, 23, 24, 25, 26
    };

    ASSERT(INITIAL_MAP[mInitial - L'ㄱ'] != INVALID);
    ASSERT(combinedFinal == 0 || FINAL_MAP[combinedFinal - L'ㄱ'] != INVALID);

    return L'가' +
        util::MEDIAL_COUNT * util::FINAL_COUNT * INITIAL_MAP[mInitial - L'ㄱ'] +
        util::FINAL_COUNT * (combinedMedial - L'ㅏ') +
        (combinedFinal ? FINAL_MAP[combinedFinal - L'ㄱ'] + 1 : 0);
}

bool Composition::CanCombineAlphabets(const wchar_t a, const wchar_t b)
{
    if (a == 0 || b == 0)
    {
        return true;
    }

    switch (a)
    {
    case L'ㄱ':
        return b == L'ㅅ';

    case L'ㄴ':
        return b == L'ㅈ' || b == L'ㅎ';

    case L'ㄹ':
        return b == L'ㄱ' || b == L'ㅁ' || b == L'ㅂ' || b == L'ㅅ' || b == L'ㅌ' || b == L'ㅍ' || b == L'ㅎ';

    case L'ㅂ':
        return b == L'ㅅ';

    case L'ㅗ':
        return b == L'ㅏ' || b == L'ㅐ' || b == L'ㅣ';

    case L'ㅜ':
        return b == L'ㅓ' || b == L'ㅔ' || b == L'ㅣ';

    case L'ㅡ':
        return b == L'ㅣ';

    default:
        return false;
    }
}

bool Composition::IsValidForFinal(wchar_t consonant)
{
    ASSERT(util::is_hangeul_consonant(consonant));

    return consonant != L'ㄸ' && consonant != L'ㅃ' && consonant != L'ㅉ';
}

wchar_t Composition::CombineAlphabets(const wchar_t a, const wchar_t b)
{
    ASSERT(CanCombineAlphabets(a, b));

    if (a == 0)
    {
        return b;
    }
    if (b == 0)
    {
        return a;
    }

    switch (a)
    {
    case L'ㄱ':
        return L'ㄳ';

    case L'ㄴ':
        return b == L'ㅈ' ? L'ㄵ' : L'ㄶ';

    case L'ㄹ':
        switch (b)
        {
        case L'ㄱ':
            return L'ㄺ';

        case L'ㅁ':
            return L'ㄻ';

        case L'ㅂ':
            return L'ㄼ';

        case L'ㅅ':
            return L'ㄽ';

        case L'ㅌ':
            return L'ㄾ';

        case L'ㅍ':
            return L'ㄿ';

        case L'ㅎ':
            return L'ㅀ';

        default:
            std::unreachable();
        }

    case L'ㅂ':
        return L'ㅄ';

    case L'ㅗ':
        switch (b)
        {
        case L'ㅏ':
            return L'ㅘ';

        case L'ㅐ':
            return L'ㅙ';

        case L'ㅣ':
            return L'ㅚ';

        default:
            std::unreachable();
        }

    case L'ㅜ':
        switch (b)
        {
        case L'ㅓ':
            return L'ㅝ';

        case L'ㅔ':
            return L'ㅞ';

        case L'ㅣ':
            return L'ㅟ';

        default:
            std::unreachable();
        }

    case L'ㅡ':
        return L'ㅢ';

    default:
        std::unreachable();
    }
}

bool Composition::removeAlphabet()
{
    if (mFinal[1] != 0)
    {
        mFinal[1] = 0;
    }
    else if (mFinal[0] != 0)
    {
        mFinal[0] = 0;
    }
    else if (mMedial[1] != 0)
    {
        mMedial[1] = 0;
    }
    else if (mMedial[0] != 0)
    {
        mMedial[0] = 0;
    }
    else if (mInitial != 0)
    {
        mInitial = 0;
    }
    else
    {
        return false;
    }

    return true;
}

wchar_t Composition::composeAndReset()
{
    const wchar_t letter = ComposeLetter();
    mInitial = 0;
    mMedial[0] = 0;
    mMedial[1] = 0;
    mFinal[0] = 0;
    mFinal[1] = 0;
    return letter;
}

void Composition::composeAndResetAndRunCallback()
{
    if (const wchar_t letter = composeAndReset();
        letter != 0)
    {
        mCompositeOutputCallback(letter);
    }
}

}
