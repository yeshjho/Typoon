#include "imm_simulator.h"

#include "../utils/logger.h"


void ImmSimulator::AddLetter(wchar_t letter, bool doMulticast)
{
    InputMessage messages[MAX_INPUT_COUNT];
    int messageLength = 0;
    const auto lambdaAddMessage = [&messages, &messageLength](const InputMessage& message)
    {
        if (message.letter != 0)
        {
            if (messageLength >= MAX_INPUT_COUNT)
            {
                throw;
            }
            messages[messageLength++] = message;
        }
    };

    mComposition.AddLetter(letter, [&lambdaAddMessage](wchar_t letter) { lambdaAddMessage({ .letter = letter, .isBeingComposed = false }); });

    if (doMulticast)
    {
        lambdaAddMessage({ .letter = mComposition.ComposeLetter(), .isBeingComposed = true });
        multicastInput(messages, messageLength);
    }

    logger.Log(ELogLevel::DEBUG, "Composite:", mComposition.ComposeLetter());
}


bool ImmSimulator::RemoveLetter()
{
    const bool removed = mComposition.RemoveLetter();
    logger.Log(ELogLevel::DEBUG, "Composite:", mComposition.ComposeLetter());
    return removed;
}


InputMessage ImmSimulator::composeEmitResetComposition()
{
    const wchar_t letter = mComposition.ComposeLetter();
    ClearComposition();
    return { .letter = letter, .isBeingComposed = false };
}


void ImmSimulator::multicastInput(const InputMessage(&messages)[MAX_INPUT_COUNT], int length) const
{
    mInputMulticastFunc(messages, length);
}


void ImmSimulator::ClearComposition()
{
    if (mComposition.ComposeLetter() != 0)
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


void setup_imm_simulator()
{
}


void teardown_imm_simulator()
{
    imm_simulator.ClearComposition();
}
