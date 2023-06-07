#pragma once
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

    // Compose a letter with the current composition.
    [[nodiscard]] wchar_t ComposeLetter() const;

private:
    [[nodiscard]] static bool canCombineLetters(wchar_t a, wchar_t b);
    [[nodiscard]] static bool canBeAFinalLetter(wchar_t consonant);
    // Combines two letters into one. If one of the letters is 0, returns the other letter.
    // It assumes that two letters are combineable. (i.e., Call canCombineLetters first.)
    [[nodiscard]] static wchar_t combineLetters(wchar_t a, wchar_t b);

    InputMessage composeEmitResetComposition();


private:
    // Korean letter composition.
    // ex - 곿->initial : ㄱ, medial : ㅗ ㅏ, final : ㄱ ㅅ
    // 꺠->initial : ㄲ, medial : ㅒ 0, final : 0 0
    // ㅏ->initial : 0, medial : ㅏ 0, final : 0 0
    struct Composition
    {
        wchar_t initial = 0;
        wchar_t medial[2] = { 0, };
        wchar_t final[2] = { 0, };
    };

    Composition mComposition;
};


void setup_imm_simulator();
void teardown_imm_simulator();


inline ImmSimulator imm_simulator{};
