#pragma once
#include <functional>


// Korean letter composition.
// ex - 곿->initial : ㄱ, medial : ㅗ ㅏ, final : ㄱ ㅅ
// 꺠->initial : ㄲ, medial : ㅒ 0, final : 0 0
// ㅏ->initial : 0, medial : ㅏ 0, final : 0 0
struct Composition
{
    wchar_t initial = 0;
    wchar_t medial[2] = { 0, };
    wchar_t final[2] = { 0, };

    void AddLetter(wchar_t letter, const std::function<void(wchar_t)>& compositeOutputCallback = nullptr);
    // Returns whether a letter was removed from it or not.
    bool RemoveLetter();

    // Compose a letter with the current composition.
    [[nodiscard]] wchar_t ComposeLetter() const;

    [[nodiscard]] static bool CanCombineLetters(wchar_t a, wchar_t b);
    [[nodiscard]] static bool CanBeAFinalLetter(wchar_t consonant);
    // Combines two letters into one. If one of the letters is 0, returns the other letter.
    // It assumes that two letters are combineable. (i.e., Call canCombineLetters first.)
    [[nodiscard]] static wchar_t CombineLetters(wchar_t a, wchar_t b);

private:
    wchar_t composeAndReset();
};
