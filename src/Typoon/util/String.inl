#pragma once


namespace typoon::util
{
    constexpr bool is_latin_alphabet(const wchar_t c)
    {
        return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
    }
}
