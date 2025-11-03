#pragma once
#include "util/Function.h"


namespace typoon::core
{

/**
 * @brief 한글 낱자를 입력받아 조합된 글자로 출력하는 클래스. 두벌식 자판으로 한글을 입력할 때와 동일하게 작동함.
 */
class Composition
{
public:
    /**
     * @param compositeOutputCallback 글자가 조합되어 출력될 때 호출되는 콜백
     */
    explicit Composition(util::NullableCallback<wchar_t> compositeOutputCallback = {});

public:
    /**
     * @brief 낱자를 하나 추가함
     * @param alphabet 추가할 낱자. 한글이 아니어도 됨.
     * @details 낱자가 추가되어 조합이 끝나면, 즉 기존 글자에 해당 낱자를 더할 수 없을 때, 콜백이 기존 글자와 함께 호출되고 새 글자가 해당 낱자로 시작됨.
     * @note '\b' 문자의 경우 현재 조합이 비어 있을 경우 콜백이 호출되고, 비어 있지 않을 경우 '마지막' 낱자가 제거됨.
     */
    void AddAlphabet(wchar_t alphabet);

    /**
     * @brief 현재 상태로 글자를 조합해 반환함
     * @return 조합된 글자
     * @note 아무 낱자도 없는 상태일 경우 '\0'을 반환함.
     */
    [[nodiscard]] wchar_t ComposeLetter() const;

public:
    /**
     * @brief 두 낱자가 조합 가능한지 여부를 확인
     * @param a 첫 번째 낱자, '\0'일 수 있음
     * @param b 두 번째 낱자, '\0'일 수 있음
     * @note '\0'은 어떤 낱자와도 조합 가능함.
     */
    [[nodiscard]] static bool CanCombineAlphabets(wchar_t a, wchar_t b);

    /**
     * @brief 자음이 종성으로 유효한지 여부를 확인
     * @param consonant 확인할 자음. 한글 자음이어야 함.
     */
    [[nodiscard]] static bool IsValidForFinal(wchar_t consonant);

    /**
     * @brief 두 낱자를 조합함
     * @param a 첫 번째 낱자, '\0'일 수 있음
     * @param b 두 번째 낱자, '\0'일 수 있음
     * @return 조합된 낱자
     * @note '\0'은 어떤 낱자와도 조합 가능함.
     * @warning 두 낱자가 조합 가능하다고 가정함. CanCombineAlphabets()로 확인한 후에 호출할 것.
     */
    [[nodiscard]] static wchar_t CombineAlphabets(wchar_t a, wchar_t b);

private:
    /**
     * @brief 낱자를 하나 제거함
     * @return 낱자가 제거되었는지 여부
     */
    bool removeAlphabet();

    /**
     * @brief 현재 상태로 글자를 조합해 반환하고 상태를 초기화함
     * @return 조합된 글자
     */
    wchar_t composeAndReset();

    /**
     * @brief composeAndReset() 호출 후 리턴 값이 '\0'이 아니라면 콜백을 실행함
     */
    void composeAndResetAndRunCallback();


private:
    /*
     * 곿 -> initial: ㄱ, medial: ㅗ ㅏ, final: ㄱ ㅅ
     * 꺠 -> initial: ㄲ, medial: ㅒ 0, final: 0 0
     * ㄴ -> initial: ㄴ, medial: 0 0, final: 0 0
     * ㅏ -> initial: 0, medial: ㅏ 0, final: 0 0
     * ㄵ -> initial: 0, medial: 0 0, final: ㄴ ㅈ
     */
    wchar_t mInitial = 0;
    wchar_t mMedial[2] = { };
    wchar_t mFinal[2] = { };

    util::NullableCallback<wchar_t> mCompositeOutputCallback;
};

}
