#include "imm_simulator.h"

#include "../utils/logger.h"


void ImmSimulator::AddLetter(wchar_t letter, bool doMulticast)
{
    InputMessage messages[MAX_INPUT_COUNT];
    int messageLength = 0;
    const auto lambdaAddMessage = [&messages, &messageLength](const InputMessage& message)
    {
        if (messageLength >= MAX_INPUT_COUNT)
        {
            throw;
        }

        if (message.letter != 0)
        {
            messages[messageLength++] = message;
        }
    };

    if (letter == '\b')
    {
        if (!RemoveLetter())
        {
            lambdaAddMessage({ letter, false });
            multicastInput(messages, messageLength);
        }
        // We don't need to multicast the input at all if a letter is successfully removed from the composition,
        // since the after-backspace-composition has already been cast for trigger checking before the backspace.

        logger.Log(ELogLevel::DEBUG, "Composite:", ComposeLetter());
        return;
    }

    const bool isConsonant = L'ㄱ' <= letter && letter <= L'ㅎ';
    const bool isVowel = L'ㅏ' <= letter && letter <= L'ㅣ';

    // If the letter is not a Korean letter, emit the current composition and then emit the letter.
    if (!isConsonant && !isVowel)
    {
        lambdaAddMessage(composeEmitResetComposition());
        lambdaAddMessage({ letter, false });
        multicastInput(messages, messageLength);

        logger.Log(ELogLevel::DEBUG, "Composite:", ComposeLetter());
        return;
    }

    if (isConsonant)
    {
        // The double final consonants('ㄳ', 'ㄵ', 'ㄶ', ...) can be a letter on their own.
        // So if there is a initial letter but no medial letter and the letter can be combined with the initial letter,
        // Combine those two and set them as the final letter. The letter will be a only-final-letter.
        if (const bool hasInitial = mComposition.initial != 0, hasMedial = mComposition.medial[0] != 0;
            hasInitial && !hasMedial && canCombineLetters(mComposition.initial, letter))
        {
            mComposition.final[0] = mComposition.initial;
            mComposition.final[1] = letter;
            mComposition.initial = 0;
        }
        // Since there is no initial requires two key strokes to be composed, if there is no initial letter, we can emit the previous and set the letter as the initial.
        // Why emit and set instead of setting right away? There are compositions which only contain medial letters. (ex - 'ㅏ' followed by 'ㄱ')
        // If there is no medial letter, we can also know that the previous composition is finished. (ex - 'ㄱ' followed by 'ㄱ')
        // If the final letters are full(ex - '갃' followed by 'ㄱ') or the letter cannot be combined with the previous final letter(ex - '간' followed by 'ㄱ'),
        // the composition is finished.
        else if (!hasInitial || !hasMedial || !canBeAFinalLetter(letter) || mComposition.final[1] != 0 || !canCombineLetters(mComposition.final[0], letter))
        {
            lambdaAddMessage(composeEmitResetComposition());
            mComposition.initial = letter;
        }
        // There is an initial letter, at least one medial letter, and the letter can be combined with the previous final letter (or there was no final letter).
        else
        {
            mComposition.final[mComposition.final[0] == 0 ? 0 : 1] = letter;
        }
    }
    else  // isVowel
    {
        // If there is at least one letter in the final, detach the latter one and use that as a new letter's initial. (ex - '각' followed by 'ㅑ' becomes '가갸')
        // This is also the reason why we can't emit a composition right away even if there are no letters left that can be added to the composition.
        if (mComposition.final[0] != 0)
        {
            wchar_t& consonantToDetach = mComposition.final[mComposition.final[1] == 0 ? 0 : 1];
            const wchar_t newInitial = consonantToDetach;
            consonantToDetach = 0;
            lambdaAddMessage(composeEmitResetComposition());
            mComposition.initial = newInitial;
            mComposition.medial[0] = letter;
        }
        // If the medial letters are full(ex - '과' followed by 'ㅏ') or the letter cannot be combined with the previous medial letter(ex - '구' followed by 'ㅏ'),
        // Emit the previous composition and set the letter as the medial (It'll become a medial-only letter, with no initial letter).
        else if (mComposition.medial[1] != 0 || !canCombineLetters(mComposition.medial[0], letter))
        {
            lambdaAddMessage(composeEmitResetComposition());
            mComposition.medial[0] = letter;
        }
        // There is no final letter, and the letter can be combined with the previous medial letter (or there was no medial letter).
        else
        {
            mComposition.medial[mComposition.medial[0] == 0 ? 0 : 1] = letter;
        }
    }

    if (doMulticast)
    {
        lambdaAddMessage({ ComposeLetter(), true });
        multicastInput(messages, messageLength);
    }

    logger.Log(ELogLevel::DEBUG, "Composite:", ComposeLetter());
}


bool ImmSimulator::RemoveLetter()
{
    if (mComposition.final[1] != 0)
    {
        mComposition.final[1] = 0;
    }
    else if (mComposition.final[0] != 0)
    {
        mComposition.final[0] = 0;
    }
    else if (mComposition.medial[1] != 0)
    {
        mComposition.medial[1] = 0;
    }
    else if (mComposition.medial[0] != 0)
    {
        mComposition.medial[0] = 0;
    }
    else if (mComposition.initial != 0)
    {
        mComposition.initial = 0;
    }
    else
    {
        logger.Log(ELogLevel::DEBUG, "Composite:", ComposeLetter());
        return false;
    }

    logger.Log(ELogLevel::DEBUG, "Composite:", ComposeLetter());
    return true;
}


InputMessage ImmSimulator::composeEmitResetComposition()
{
    const wchar_t letter = ComposeLetter();
    ClearComposition();
    return { letter, false };
}


void ImmSimulator::multicastInput(const InputMessage(&messages)[MAX_INPUT_COUNT], int length)
{
    mInputMulticastFunc(messages, length);
}


void ImmSimulator::ClearComposition()
{
    if (ComposeLetter() != 0)
    {
        logger.Log(ELogLevel::DEBUG, "Reset Composite");
    }
    mComposition = {};
}


void ImmSimulator::EmitAndClearCurrentComposite()
{
    const InputMessage messages[MAX_INPUT_COUNT] = { composeEmitResetComposition() };
    if (messages[0].letter != 0)
    {
        multicastInput(messages, 1);
    }
}


wchar_t ImmSimulator::ComposeLetter() const
{
    const wchar_t medial = combineLetters(mComposition.medial[0], mComposition.medial[1]);
    const wchar_t final = combineLetters(mComposition.final[0], mComposition.final[1]);

    // If there is no initial or medial letter, it can be a final-only letter.
    if (const bool hasInitial = mComposition.initial != 0, hasMedial = medial != 0;
        !hasInitial && !hasMedial)
    {
        return final;
    }
    // If there is no initial letter, it can only be a medial-only letter.
    else if (!hasInitial)
    {
        return medial;
    }
    // If there is no medial letter, it can only be an initial-only letter.
    else if (!hasMedial)
    {
        return mComposition.initial;
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
        MEDIAL_COUNT * FINAL_COUNT * INITIAL_MAP[mComposition.initial - L'ㄱ'] +
        FINAL_COUNT * (medial - L'ㅏ') +
        (final ? FINAL_MAP[final - L'ㄱ'] + 1 : 0);  // There can be no final letter at all
}


bool ImmSimulator::canCombineLetters(wchar_t a, wchar_t b)
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


bool ImmSimulator::canBeAFinalLetter(wchar_t consonant)
{
    return consonant != L'ㄸ' && consonant != L'ㅃ' && consonant != L'ㅉ';
}


wchar_t ImmSimulator::combineLetters(wchar_t a, wchar_t b)
{
    if (a == 0)
    {
        return b;
    }
    if (b == 0)
    {
        return a;
    }

    // It assumes that two letters are combineable. (Use canCombineLetters() to check)
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


void setup_imm_simulator()
{
}

void teardown_imm_simulator()
{
    imm_simulator.ClearComposition();
}
