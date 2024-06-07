#pragma once
#include <string>


namespace json5
{
struct error;
}


// Whether the character is alphabetic and has a case.
bool is_cased_alpha(wchar_t c);

std::wstring to_u16_string(const std::string& str);

std::string to_u8_string(const std::wstring& str);

// Separate all the letters in each Korean letter into consonants and vowels.
// ex - '곿까ㅒㄷ' -> 'ㄱㅗㅏㄱㅅㄲㅏㅒㄷ'
std::wstring normalize_hangeul(std::wstring_view str);

// Exact opposite of `normalize_hangeul`.
// ex - ㄱㅗㅏㄱㅅㄲㅏㅒㄷ' -> '곿까ㅒㄷ'
std::wstring combine_hangeul(std::wstring_view str);

std::wstring alphabet_to_hangeul(std::wstring_view str);

std::wstring hangeul_to_alphabet(std::wstring_view normalizedStr, bool isCapsLockOn);

constexpr bool is_korean(wchar_t c);

std::wstring json5_error_to_string(const json5::error& err);


constexpr bool is_korean(wchar_t c)
{
    return
        (L'가' <= c && c <= L'힣') ||
        (L'ㄱ' <= c && c <= L'ㅣ');
}
