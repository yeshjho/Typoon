#pragma once
#include "composition.h"
#include "../input_multicast/input_multicast.h"


class ImmSimulator
{
public:
    void AddLetter(wchar_t letter, bool doMulticast = true);
    // Returns whether a letter was removed from the current composition.
    bool RemoveLetter();
    
    // Called when the composition is finished by either
    // 1. Adding another letter
    // 2. Adding non-Korean letter
    // 3. A non-letter key (ex - Korean/English toggle key)
    // 4. A mouse input
    void ClearComposition();

    void EmitAndClearCurrentComposite();

    void RedirectInputMulticast(std::function<void(const InputMessage(&messages)[MAX_INPUT_COUNT], int length)> func) { mInputMulticastFunc = std::move(func); }

private:
    InputMessage composeEmitResetComposition();

    void multicastInput(const InputMessage (&messages)[MAX_INPUT_COUNT], int length);


private:
    Composition mComposition;

    std::function<void(const InputMessage(&messages)[MAX_INPUT_COUNT], int length)> mInputMulticastFunc = [](const InputMessage(&messages)[MAX_INPUT_COUNT], int length) { multicast_input(messages, length); };
};


void setup_imm_simulator();
void teardown_imm_simulator();


inline ImmSimulator imm_simulator{};
