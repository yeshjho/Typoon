#pragma once
#include <string>


// Whether the character is alphabetic and has a case.
bool is_cased_alpha(wchar_t c);

std::wstring to_u16_string(const std::string& str);

// Separate all the letters in each Korean letter into consonants and vowels.
// ex - '곿까ㅒㄷ' -> ㄱㅗㅏㄱㅅㄲㅏㅒㄷ'
std::wstring normalize_hangeul(std::wstring_view str);

std::wstring alphabet_to_hangeul(std::wstring_view str);

std::wstring hangeul_to_alphabet(std::wstring_view normalizedStr, bool isCapsLockOn);

constexpr bool is_korean(wchar_t c);


constexpr bool is_korean(wchar_t c)
{
    return
        (L'가' <= c && c <= L'힣') ||
        (L'ㄱ' <= c && c <= L'ㅣ');
}
