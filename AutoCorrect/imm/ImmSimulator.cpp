#include "ImmSimulator.h"

#include <thread>

#include <atomic_queue/atomic_queue.h>

#include "../input_multicast/input_multicast.h"
#include "../utils/logger.h"


void ImmSimulator::AddLetter(wchar_t letter)
{
    const bool isConsonant = L'ㄱ' <= letter && letter <= L'ㅎ';
    const bool isVowel = L'ㅏ' <= letter && letter <= L'ㅣ';

    if (!isConsonant && !isVowel)
    {
        ComposeEmitResetComposition();
        multicast_input({ letter, false });
        return;
    }

    if (isConsonant)
    {
        if (mComposition.initial == 0 || mComposition.medial[0] == 0 ||
            mComposition.final[1] != 0 || !canCombineLetters(mComposition.final[0], letter))
        {
            ComposeEmitResetComposition();
            mComposition.initial = letter;
        }
        else
        {
            mComposition.final[mComposition.final[0] == 0 ? 0 : 1] = letter;
        }
    }
    else  // isVowel
    {
        if (mComposition.final[0] != 0)
        {
            wchar_t& consonantToDetach = mComposition.final[mComposition.final[1] == 0 ? 0 : 1];
            const wchar_t newInitial = consonantToDetach;
            consonantToDetach = 0;
            ComposeEmitResetComposition();
            mComposition.initial = newInitial;
            mComposition.medial[0] = letter;
        }
        else if (mComposition.medial[1] != 0 || !canCombineLetters(mComposition.medial[0], letter))
        {
            ComposeEmitResetComposition();
            mComposition.medial[0] = letter;
        }
        else
        {
            mComposition.medial[mComposition.medial[0] == 0 ? 0 : 1] = letter;
        }
    }

    multicast_input({ composeLetter(), true });
}


void ImmSimulator::ComposeEmitResetComposition()
{
    if (const wchar_t letter = composeLetter();
        letter != 0)
    {
        multicast_input({ letter, false });
    }
    ClearComposition();
}


void ImmSimulator::ClearComposition()
{
    mComposition = {};
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
            throw;
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
            throw;
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
            throw;
        }

    case L'ㅡ':
        return L'ㅢ';

    default:
        throw;
    }
}


wchar_t ImmSimulator::composeLetter() const
{
    const wchar_t medial = combineLetters(mComposition.medial[0], mComposition.medial[1]);

    if (mComposition.initial == 0)
    {
        return medial;
    }
    if (medial == 0)
    {
        return mComposition.initial;
    }

    const wchar_t final = combineLetters(mComposition.final[0], mComposition.final[1]);
    
    constexpr wchar_t MEDIAL_COUNT = L'ㅣ' - L'ㅏ' + 1;
    constexpr wchar_t FINAL_COUNT = L'갛' - L'가' + 1;
    constexpr wchar_t INVALID = 0xFFFF;

    // ㄱ ㄲ ㄳ ㄴ ㄵ ㄶ ㄷ ㄸ ㄹ ㄺ ㄻ ㄼ ㄽ ㄾ ㄿ ㅀ ㅁ ㅂ ㅃ ㅄ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ
    constexpr wchar_t INITIAL_MAP[] = {
        0, 1, INVALID, 2, INVALID, INVALID, 3, 4, 5, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, 6, 7, 8, INVALID, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
    };
    constexpr wchar_t FINAL_MAP[] = {
        0, 1, 2, 3, 4, 5, 6, INVALID, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, INVALID, 17, 18, 19, 20, 21, INVALID, 22, 23, 24, 25, 26
    };

    return L'가' + 
        MEDIAL_COUNT * FINAL_COUNT * INITIAL_MAP[mComposition.initial - L'ㄱ'] +
        FINAL_COUNT * (medial - L'ㅏ') +
        (final ? FINAL_MAP[final - L'ㄱ'] + 1 : 0);
}


using RawInputQueueType = atomic_queue::AtomicQueue<wchar_t, 20, atomic_queue::details::nil<wchar_t>(), true, true, true, true>;
RawInputQueueType raw_input_queue;
std::jthread imm_simulator_thread;


void setup_imm_simulator()
{
    if (imm_simulator_thread.joinable()) [[unlikely]]
    {
        g_console_logger.Log("Imm Simulator is already running.", ELogLevel::WARNING);
        return;
    }

    imm_simulator_thread = std::jthread{ [&queue = raw_input_queue, simulator = ImmSimulator{}](const std::stop_token& stopToken) mutable
    {
        while (true)
        {
            if (stopToken.stop_requested()) [[unlikely]]
            {
                break;
            }

            simulator.AddLetter(queue.pop());
        }
    } };
}

void teardown_imm_simulator()
{
    imm_simulator_thread.request_stop();
    if (imm_simulator_thread.joinable())
    {
        imm_simulator_thread.join();
    }
}

void send_raw_input_to_imm_simulator(wchar_t letter)
{
    raw_input_queue.push(letter);
}
