#include "composition.h"

#include <utility>


void Composition::AddLetter(wchar_t letter, const std::function<void(wchar_t)>& compositeOutputCallback)
{
    const auto lambdaCompositeOutputCallback = [this, compositeOutputCallback](wchar_t composite)
        {
            if (compositeOutputCallback)
            {
                compositeOutputCallback(composite);
            }
        };

    if (letter == '\b')
    {
        if (!RemoveLetter())
        {
            lambdaCompositeOutputCallback('\b');
        }
        // We don't need to multicast the input at all if a letter is successfully removed from the composition,
        // since the after-backspace-composition has already been cast for trigger checking before the backspace.
        return;
    }

    const bool isConsonant = L'ㄱ' <= letter && letter <= L'ㅎ';
    const bool isVowel = L'ㅏ' <= letter && letter <= L'ㅣ';

    // If the letter is not a Korean letter, emit the current composition and then emit the letter.
    if (!isConsonant && !isVowel)
    {
        lambdaCompositeOutputCallback(composeAndReset());
        lambdaCompositeOutputCallback(letter);
        return;
    }

    if (isConsonant)
    {
        // The double final consonants('ㄳ', 'ㄵ', 'ㄶ', ...) can be a letter on their own.
        // So if there is a initial letter but no medial letter and the letter can be combined with the initial letter,
        // Combine those two and set them as the final letter. The letter will be a only-final-letter.
        if (const bool hasInitial = initial != 0, hasMedial = medial[0] != 0;
            hasInitial && !hasMedial && CanCombineLetters(initial, letter))
        {
            final[0] = initial;
            final[1] = letter;
            initial = 0;
        }
        // Since there is no initial requires two key strokes to be composed, if there is no initial letter, we can emit the previous and set the letter as the initial.
        // Why emit and set instead of setting right away? There are compositions which only contain medial letters. (ex - 'ㅏ' followed by 'ㄱ')
        // If there is no medial letter, we can also know that the previous composition is finished. (ex - 'ㄱ' followed by 'ㄱ')
        // If the final letters are full(ex - '갃' followed by 'ㄱ') or the letter cannot be combined with the previous final letter(ex - '간' followed by 'ㄱ'),
        // the composition is finished.
        else if (!hasInitial || !hasMedial || !CanBeAFinalLetter(letter) || final[1] != 0 || !CanCombineLetters(final[0], letter))
        {
            lambdaCompositeOutputCallback(composeAndReset());
            initial = letter;
        }
        // There is an initial letter, at least one medial letter, and the letter can be combined with the previous final letter (or there was no final letter).
        else
        {
            final[final[0] == 0 ? 0 : 1] = letter;
        }
    }
    else  // isVowel
    {
        // If there is at least one letter in the final, detach the latter one and use that as a new letter's initial. (ex - '각' followed by 'ㅑ' becomes '가갸')
        // This is also the reason why we can't emit a composition right away even if there are no letters left that can be added to the composition.
        if (final[0] != 0)
        {
            wchar_t& consonantToDetach = final[final[1] == 0 ? 0 : 1];
            const wchar_t newInitial = consonantToDetach;
            consonantToDetach = 0;
            lambdaCompositeOutputCallback(composeAndReset());
            initial = newInitial;
            medial[0] = letter;
        }
        // If the medial letters are full(ex - '과' followed by 'ㅏ') or the letter cannot be combined with the previous medial letter(ex - '구' followed by 'ㅏ'),
        // Emit the previous composition and set the letter as the medial (It'll become a medial-only letter, with no initial letter).
        else if (medial[1] != 0 || !CanCombineLetters(medial[0], letter))
        {
            lambdaCompositeOutputCallback(composeAndReset());
            medial[0] = letter;
        }
        // There is no final letter, and the letter can be combined with the previous medial letter (or there was no medial letter).
        else
        {
            medial[medial[0] == 0 ? 0 : 1] = letter;
        }
    }
}


bool Composition::RemoveLetter()
{
    if (final[1] != 0)
    {
        final[1] = 0;
    }
    else if (final[0] != 0)
    {
        final[0] = 0;
    }
    else if (medial[1] != 0)
    {
        medial[1] = 0;
    }
    else if (medial[0] != 0)
    {
        medial[0] = 0;
    }
    else if (initial != 0)
    {
        initial = 0;
    }
    else
    {
        return false;
    }

    return true;
}


wchar_t Composition::ComposeLetter() const
{
    const wchar_t combinedMedial = CombineLetters(medial[0], medial[1]);
    const wchar_t combinedFinal = CombineLetters(final[0], final[1]);

    // If there is no initial or medial letter, it can be a final-only letter.
    if (const bool hasInitial = initial != 0, hasMedial = combinedMedial != 0;
        !hasInitial && !hasMedial)
    {
        return combinedFinal;
    }
    // If there is no initial letter, it can only be a medial-only letter.
    else if (!hasInitial)
    {
        return combinedMedial;
    }
    // If there is no medial letter, it can only be an initial-only letter.
    else if (!hasMedial)
    {
        return initial;
    }

    constexpr wchar_t MEDIAL_COUNT = L'ㅣ' - L'ㅏ' + 1;
    constexpr wchar_t FINAL_COUNT = L'갛' - L'가' + 1;
    constexpr wchar_t INVALID = 0xFFFF;

    // All possible consonant combinations are (in Unicode-order):
    // ㄱ ㄲ ㄳ ㄴ ㄵ ㄶ ㄷ ㄸ ㄹ ㄺ ㄻ ㄼ ㄽ ㄾ ㄿ ㅀ ㅁ ㅂ ㅃ ㅄ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ (0x3131 ~ 0x314E)
    // But not all combinations can be used as an initial nor a final.
    // The composed letters, starting from 0xAC00, of course skips the invalid ones. So we need to map appropriately.
    constexpr wchar_t INITIAL_MAP[] = {
        0, 1, INVALID, 2, INVALID, INVALID, 3, 4, 5, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, 6, 7, 8, INVALID, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
    };
    constexpr wchar_t FINAL_MAP[] = {
        0, 1, 2, 3, 4, 5, 6, INVALID, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, INVALID, 17, 18, 19, 20, 21, INVALID, 22, 23, 24, 25, 26
    };

    return L'가' +
        MEDIAL_COUNT * FINAL_COUNT * INITIAL_MAP[initial - L'ㄱ'] +
        FINAL_COUNT * (combinedMedial - L'ㅏ') +
        (combinedFinal ? FINAL_MAP[combinedFinal - L'ㄱ'] + 1 : 0);  // There can be no final letter at all
}


bool Composition::CanCombineLetters(wchar_t a, wchar_t b)
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


bool Composition::CanBeAFinalLetter(wchar_t consonant)
{
    return consonant != L'ㄸ' && consonant != L'ㅃ' && consonant != L'ㅉ';
}


wchar_t Composition::CombineLetters(wchar_t a, wchar_t b)
{
    if (a == 0)
    {
        return b;
    }
    if (b == 0)
    {
        return a;
    }

    // It assumes that two letters are combineable. (Use CanCombineLetters() to check)
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


wchar_t Composition::composeAndReset()
{
    const wchar_t letter = ComposeLetter();
    *this = {};
    return letter;
}