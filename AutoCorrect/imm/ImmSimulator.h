#pragma once


class ImmSimulator
{
public:
    void AddLetter(wchar_t letter);
    void RemoveLetter();  // TODO

    /*!
     * @brief Called when the composition is finished by either
     * 1. Adding another letter
     * 2. Adding non-Korean letter
     * 3. A non-letter key (ex - Korean/English toggle key)
     * 4. A mouse input
    */
    void ComposeEmitResetComposition();
    void ClearComposition();

private:
    [[nodiscard]] static bool canCombineLetters(wchar_t a, wchar_t b);
    /*!
     * @brief Combines two letters into one. If one of the letters is 0, returns the other letter.
     * It assumes that two letters are combineable. (i.e., Call canCombineLetters first.)
    */
    [[nodiscard]] static wchar_t combineLetters(wchar_t a, wchar_t b);

    [[nodiscard]] wchar_t composeLetter() const;


private:
    struct Composition
    {
        wchar_t initial = 0;
        wchar_t medial[2] = { 0, };
        wchar_t final[2] = { 0, };
    };

    Composition mComposition;
};
