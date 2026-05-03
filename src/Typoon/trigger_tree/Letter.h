#pragma once
#include <compare>

#ifdef __cpp_lib_inplace_vector 
#include <inplace_vector>
#else
#include <vector>
namespace std
{
    template<typename T, size_t N>
    using inplace_vector = std::vector<T>;
}
#endif


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
        explicit Letter(wchar_t letter, bool isCaseSensitive = false, bool doNeedFullComposite = false);

    public:
        // std::map에서 key값 비교할 때 사용.
        bool operator<(const Letter& other) const;

        // Matcher에서 Input과 비교할 때 사용.
        bool operator==(wchar_t ch) const;

        // 이 Letter의 wchar_t와의 동등관계에 대한 포함집합 Letter들을 반환. 즉 이 Letter와 ==한 모든 wchar_t에 대해 똑같이 ==한 Letter들의 모음.
        // mDoNeedFullComposite은 ==(wchar_t)에 영향을 미치지 않으므로 여기에서도 제외됨.
        // 트리거의 도달 가능성 여부를 판단할 때 사용.
        [[nodiscard]] std::inplace_vector<Letter, 2> GetSupersetLetters() const;
        // GetSupersetLetters 참고. 부분집합을 반환.
        // 단, NON_WORD_LETTER의 경우 원소가 너무 많고, 사용 용도에 있어 의미가 없기 때문에 빈 배열을 반환함.
        [[nodiscard]] std::inplace_vector<Letter, 3> GetSubsetLetters() const;

        // Matcher에서 Input과 equivalent한 글자들 범위 구할 때 사용.
        bool operator<(wchar_t ch) const;
        friend bool operator<(wchar_t ch, const Letter& letter);

        bool operator==(const Letter&) const = default;

    public:
        [[nodiscard]] wchar_t GetLetter() const { return mLetter; }
        [[nodiscard]] wchar_t GetLetterLowered() const { return mLetterLowered; }
        [[nodiscard]] bool IsCaseSensitive() const { return mIsCaseSensitive; }
        [[nodiscard]] bool DoNeedFullComposite() const { return mDoNeedFullComposite; }
        [[nodiscard]] bool IsSpecial() const { return mIsSpecial; }

    private:
        bool mIsCaseSensitive = false;
        wchar_t mLetterLowered = 0;
        wchar_t mLetter = 0;  // mIsCaseSensitive가 false이면 mLetterLowered와 동일
        bool mDoNeedFullComposite = false;
        bool mIsSpecial = false;
    };


    bool operator<(wchar_t ch, const Letter& letter);
}
