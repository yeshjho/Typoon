#include "Hangeul.h"


bool is_hangeul_consonant(const wchar_t ch)
{
    return L'ㄱ' <= ch && ch <= L'ㅎ';
}

bool is_hangeul_vowel(const wchar_t ch)
{
    return L'ㅏ' <= ch && ch <= L'ㅣ';
}

bool is_hangeul_alphabet(const wchar_t ch)
{
    return is_hangeul_consonant(ch) || is_hangeul_vowel(ch);
}
