#pragma once
#include <compare>


namespace typoon::core
{

/**
 * @brief 트리거 트리의 노드
 */
class Letter
{
public:
#pragma region SpecialCharacters
    /* 특수 문자를 위한 코드포인트 할당. Private User Area에서 할당하면 다른 프로그램과 충돌할 수 있으므로 사용하면 안 됨.
     * 대신에 한글 블록 내에서 할당. 유니코드 5.2(2009년 발표) 이후로는 한글 블록에 문자가 추가되지 않았으므로 할당 가능성이 낮음.
     *
     * 후보군: {
     *     // 한글 호환 자모
     *          0x3130, 0x318F,
     *     // 한글 자모 확장 A
     *          0xA97D, 0xA97E, 0xA97F,
     *     // 한글 호환성. 문자가 할당될 가능성이 제일 낮은 범위.
     *          (0xD7A4), 0xD7A5, 0xD7A6, 0xD7A7, 0xD7A8, 0xD7A9, 0xD7AA, 0xD7AB, 0xD7AC, 0xD7AD, 0xD7AE, (0xD7AF),
     *     // 한글 자모 확장 B
     *          0xD7C7, 0xD7C8, 0xD7C9, 0xD7CA,
     *          0xD7FC, 0xD7FD, 0xD7FE, 0xD7FF
     * }
     */

    /// 트리거용 특수 문자
    /**
     * @brief word 옵션용. 단어 경계 문자
     */
    static constexpr wchar_t NON_WORD_LETTER = 0xD7A4;

    /// 대치 텍스트용 특수 문자
    /**
     * @brief word 옵션용. 마지막으로 입력된 문자
     */
    static constexpr wchar_t LAST_INPUT_LETTER = 0xD7AF;
#pragma endregion


public:
    Letter() = default;
    Letter(wchar_t letter, bool isCaseSensitive = false, bool doNeedFullComposite = false);

public:
    [[nodiscard]] bool operator==(wchar_t ch) const;
    [[nodiscard]] bool operator==(const Letter& other) const;

    [[nodiscard]] std::strong_ordering operator<=>(const Letter& other) const;

public:
    [[nodiscard]] wchar_t GetLetter() const { return mLetter; }
    [[nodiscard]] bool IsIsCaseSensitive() const { return mIsCaseSensitive; }
    [[nodiscard]] bool DoNeedFullComposite() const { return mDoNeedFullComposite; }

private:
    wchar_t mLetter = 0;
    wchar_t mLetterLowered = 0;
    bool mIsCased = false;
    bool mIsCaseSensitive = false;
    bool mDoNeedFullComposite = false;
};

}
