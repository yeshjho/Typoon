#pragma once


constexpr wchar_t MEDIAL_COUNT = L'ㅣ' - L'ㅏ' + 1;
constexpr wchar_t FINAL_COUNT = L'갛' - L'가' + 1;  // NOTE: 종성이 없는 경우 포함
constexpr wchar_t INITIAL_COUNT = (L'힣' - L'가' + 1) / (MEDIAL_COUNT * FINAL_COUNT);

/**
 * @brief 주어진 문자가 한글 자음인지 확인
 */
[[nodiscard]] bool is_hangeul_consonant(wchar_t ch);
/**
 * @brief 주어진 문자가 한글 모음인지 확인
 */
[[nodiscard]] bool is_hangeul_vowel(wchar_t ch);
