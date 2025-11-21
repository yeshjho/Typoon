#pragma once
#include <string>
#include <string_view>


namespace typoon::util
{

constexpr wchar_t MEDIAL_COUNT = L'ㅣ' - L'ㅏ' + 1;
constexpr wchar_t FINAL_COUNT = L'갛' - L'가' + 1;  // NOTE: 종성이 없는 경우 포함
constexpr wchar_t INITIAL_COUNT = (L'힣' - L'가' + 1) / (MEDIAL_COUNT * FINAL_COUNT);

constexpr wchar_t INITIAL_MAP[] = {
    L'ㄱ', L'ㄲ', L'ㄴ', L'ㄷ', L'ㄸ', L'ㄹ', L'ㅁ', L'ㅂ', L'ㅃ', L'ㅅ', L'ㅆ', L'ㅇ', L'ㅈ', L'ㅉ', L'ㅊ', L'ㅋ', L'ㅌ', L'ㅍ', L'ㅎ'
};
constexpr std::wstring_view DECOMPOSED_MEDIAL_MAP[] = {
    L"ㅏ", L"ㅐ", L"ㅑ", L"ㅒ", L"ㅓ", L"ㅔ", L"ㅕ", L"ㅖ", L"ㅗ", L"ㅗㅏ", L"ㅗㅐ", L"ㅗㅣ", L"ㅛ", L"ㅜ", L"ㅜㅓ", L"ㅜㅔ", L"ㅜㅣ", L"ㅠ", L"ㅡ", L"ㅡㅣ", L"ㅣ"
};
constexpr std::wstring_view DECOMPOSED_FINAL_MAP[] = {
    L"", L"ㄱ", L"ㄲ", L"ㄱㅅ", L"ㄴ", L"ㄴㅈ", L"ㄴㅎ", L"ㄷ", L"ㄹ", L"ㄹㄱ", L"ㄹㅁ", L"ㄹㅂ", L"ㄹㅅ", L"ㄹㅌ", L"ㄹㅍ", L"ㄹㅎ", L"ㅁ", L"ㅂ", L"ㅂㅅ", L"ㅅ", L"ㅆ", L"ㅇ", L"ㅈ", L"ㅊ", L"ㅋ", L"ㅌ", L"ㅍ", L"ㅎ"
};
constexpr std::wstring_view DECOMPOSED_CONSONANT_MAP[] = {
    L"ㄱ", L"ㄲ", L"ㄱㅅ", L"ㄴ", L"ㄴㅈ", L"ㄴㅎ", L"ㄷ", L"ㄸ", L"ㄹ", L"ㄹㄱ", L"ㄹㅁ", L"ㄹㅂ", L"ㄹㅅ", L"ㄹㅌ", L"ㄹㅍ", L"ㄹㅎ", L"ㅁ", L"ㅂ", L"ㅃ", L"ㅂㅅ", L"ㅅ", L"ㅆ", L"ㅇ", L"ㅈ", L"ㅉ", L"ㅊ", L"ㅋ", L"ㅌ", L"ㅍ", L"ㅎ"
};

constexpr wchar_t LOWER_LATIN_ALPHABET_TO_HANGEUL[] = L"ㅁㅠㅊㅇㄷㄹㅎㅗㅑㅓㅏㅣㅡㅜㅐㅔㅂㄱㄴㅅㅕㅍㅈㅌㅛㅋ";
constexpr wchar_t UPPER_LATIN_ALPHABET_TO_HANGEUL[] = L"ㅁㅠㅊㅇㄸㄹㅎㅗㅑㅓㅏㅣㅡㅜㅒㅖㅃㄲㄴㅆㅕㅍㅉㅌㅛㅋ";


/**
 * @brief 주어진 문자가 한글 자음인지 확인
 */
[[nodiscard]] bool is_hangeul_consonant(wchar_t ch);
/**
 * @brief 주어진 문자가 한글 모음인지 확인
 */
[[nodiscard]] bool is_hangeul_vowel(wchar_t ch);
/**
 * @brief 주어진 문자가 한글 낱자인지 확인
 */
[[nodiscard]] bool is_hangeul_alphabet(wchar_t ch);

/**
 * @brief 주어진 문자가 조합된 한글 글자인지 확인 (가~힣)
 */
[[nodiscard]] bool is_hangeul_composite(wchar_t ch);
/**
 * @brief 주어진 문자가 한글 글자인지 확인
 */
[[nodiscard]] bool is_hangeul(wchar_t ch);


/**
 * @brief 한글 글자를 자모 단위로 분리
 * @example '곿까ㅒㄷ' -> 'ㄱㅗㅏㄱㅅㄲㅏㅒㄷ'
 */
std::wstring decompose_hangeul(std::wstring_view str);
/**
 * @brief 한글 자모를 글자 단위로 결합
 * @example 'ㄱㅗㅏㄱㅅㄲㅏㅒㄷ' -> '곿까ㅒㄷ'
 */
std::wstring combine_hangeul(std::wstring_view str);

/**
 * @brief 라틴 알파벳 문자열을 한글 자모로 변환
 * @note 두벌식/QWERTY 자판 기준
 * @example "gksrmfQ" -> "ㅎㅏㄴㄱㅡㄹㅃ"
 */
std::wstring latin_alphabet_to_hangeul_alphabet(std::wstring_view str);
/**
 * @brief 한글 자모 문자열을 라틴 알파벳으로 변환
 * @note 두벌식/QWERTY 자판 기준
 * @example "ㅎㅏㄴㄱㅡㄹ자모ㄺ" -> "gksrmf자모ㄺ"
 */
std::wstring hangeul_alphabet_to_latin_alphabet(std::wstring_view str);

}
