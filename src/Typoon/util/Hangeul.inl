#pragma once


namespace typoon::util
{
    constexpr bool is_hangeul_consonant(const wchar_t ch)
    {
        return L'ㄱ' <= ch && ch <= L'ㅎ';
    }

    constexpr bool is_hangeul_vowel(const wchar_t ch)
    {
        return L'ㅏ' <= ch && ch <= L'ㅣ';
    }

    constexpr bool is_hangeul_alphabet(const wchar_t ch)
    {
        return is_hangeul_consonant(ch) || is_hangeul_vowel(ch);
    }

    constexpr bool is_hangeul_composite(const wchar_t ch)
    {
        return L'가' <= ch && ch <= L'힣';
    }

    constexpr bool is_hangeul(const wchar_t ch)
    {
        return is_hangeul_composite(ch) || is_hangeul_alphabet(ch);
    }
}
