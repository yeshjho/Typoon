#include "./string.h"

#include <cwctype>

#include <uni-algo/conv.h>
#include <json5/json5_base.hpp>

#include "logger.h"


bool is_cased_alpha(wchar_t c)
{
    return std::iswalpha(c) && (std::iswupper(c) ^ std::iswlower(c));
}

std::wstring to_u16_string(const std::string& str)
{
    if (una::is_valid_utf8(str))
    {
        return una::utf8to16(str);
    }
    return std::wstring{ str.begin(), str.end() };
}

std::wstring normalize_hangeul(std::wstring_view str)
{
    std::wstring result;
    result.reserve(str.size());

    for (const wchar_t c : str)
    {
        static constexpr wchar_t choseongMap[] = {
            L'ㄱ', L'ㄲ', L'ㄴ', L'ㄷ', L'ㄸ', L'ㄹ', L'ㅁ', L'ㅂ', L'ㅃ', L'ㅅ', L'ㅆ', L'ㅇ', L'ㅈ', L'ㅉ', L'ㅊ', L'ㅋ', L'ㅌ', L'ㅍ', L'ㅎ'
        };
        static constexpr std::wstring_view jungseongMap[] = {
            L"ㅏ", L"ㅐ", L"ㅑ", L"ㅒ", L"ㅓ", L"ㅔ", L"ㅕ", L"ㅖ", L"ㅗ", L"ㅗㅏ", L"ㅗㅐ", L"ㅗㅣ", L"ㅛ", L"ㅜ", L"ㅜㅓ", L"ㅜㅔ", L"ㅜㅣ", L"ㅠ", L"ㅡ", L"ㅡㅣ", L"ㅣ"
        };
        static constexpr std::wstring_view jongseongMap[] = {
            L"", L"ㄱ", L"ㄲ", L"ㄱㅅ", L"ㄴ", L"ㄴㅈ", L"ㄴㅎ", L"ㄷ", L"ㄹ", L"ㄹㄱ", L"ㄹㅁ", L"ㄹㅂ", L"ㄹㅅ", L"ㄹㅌ", L"ㄹㅍ", L"ㄹㅎ", L"ㅁ", L"ㅂ", L"ㅂㅅ", L"ㅅ", L"ㅆ", L"ㅇ", L"ㅈ", L"ㅊ", L"ㅋ", L"ㅌ", L"ㅍ", L"ㅎ"
        };
        static constexpr std::wstring_view consonantMap[] = {
            L"ㄱ", L"ㄲ", L"ㄱㅅ", L"ㄴ", L"ㄴㅈ", L"ㄴㅎ", L"ㄷ", L"ㄸ", L"ㄹ", L"ㄹㄱ", L"ㄹㅁ", L"ㄹㅂ", L"ㄹㅅ", L"ㄹㅌ", L"ㄹㅍ", L"ㄹㅎ", L"ㅁ", L"ㅂ", L"ㅃ", L"ㅂㅅ", L"ㅅ", L"ㅆ", L"ㅇ", L"ㅈ", L"ㅉ", L"ㅊ", L"ㅋ", L"ㅌ", L"ㅍ", L"ㅎ"
        };

        if (L'가' <= c && c <= L'힣')
        {
            constexpr int lettersOfAChoseong = static_cast<int>(std::size(jungseongMap) * std::size(jongseongMap));
            auto [choseong, nonChoseong] = std::div(c - L'가', lettersOfAChoseong);
            auto [jungseong, jongseong] = std::div(nonChoseong, static_cast<int>(std::size(jongseongMap)));
            result += choseongMap[choseong];
            result += jungseongMap[jungseong];
            result += jongseongMap[jongseong];
        }
        else if (L'ㄱ' <= c && c <= L'ㅎ')
        {
            result += consonantMap[c - L'ㄱ'];
        }
        else if (L'ㅏ' <= c && c <= L'ㅣ')
        {
            result += jungseongMap[c - L'ㅏ'];
        }
        // '조합형'(NFC) letters. i.e., these letters also carry the information what part(initial, medial, or final) they are.
        else if (L'ᄀ' <= c && c <= L'ᄒ') [[unlikely]]
        {
            result += choseongMap[c - L'ᄀ'];
        }
        else if (L'ᅡ' <= c && c <= L'ᅵ') [[unlikely]]
        {
            result += jungseongMap[c - L'ᅡ'];
        }
        else if (L'ᆨ' <= c && c <= L'ᇂ') [[unlikely]]
        {
            result += jongseongMap[c - L'ᆨ' + 1];  // Make up for the 'no final'.
        }
        else
        {
            result += c;
        }
    }

    return result;
}

std::wstring alphabet_to_hangeul(std::wstring_view str)
{
    std::wstring result;
    result.reserve(str.size());

    // Support only two-set keyboard layout for now.
    // a~z
    constexpr wchar_t lowerAlphabetToHangul[] = L"ㅁㅠㅊㅇㄷㄹㅎㅗㅑㅓㅏㅣㅡㅜㅐㅔㅂㄱㄴㅅㅕㅍㅈㅌㅛㅋ";
    constexpr wchar_t upperAlphabetToHangul[] = L"ㅁㅠㅊㅇㄸㄹㅎㅗㅑㅓㅏㅣㅡㅜㅒㅖㅃㄲㄴㅆㅕㅍㅉㅌㅛㅋ";

    for (const wchar_t& character : str)
    {
        if ('a' <= character && character <= 'z')
        {
            result += lowerAlphabetToHangul[character - L'a'];
        }
        else if ('A' <= character && character <= 'Z')
        {
            result += upperAlphabetToHangul[character - L'A'];
        }
        else
        {
            result += character;
        }
    }

    return result;
}

std::wstring hangeul_to_alphabet(std::wstring_view normalizedStr, bool isCapsLockOn)
{
    std::wstring result;
    result.reserve(normalizedStr.size());

    // Support only two-set keyboard layout for now.
    // ㄱ~ㅣ
    constexpr char hangeulToAlphabet[] = {
        'r', 'R', 0, 's', 0, 0, 'e', 'E', 'f', 0, 0, 0, 0, 0, 0, 0, 'a', 'q', 'Q', 0, 't', 'T', 'd', 'w', 'W', 'c', 'z', 'x', 'v', 'g',
        'k', 'o', 'i', 'O', 'j', 'p', 'u', 'P', 'h', 0, 0, 0, 'y', 'n', 0, 0, 0, 'b', 'm', 0, 'l'
    };

    for (const wchar_t& character : normalizedStr)
    {
        if (L'ㄱ' <= character && character <= L'ㅣ')
        {
            const char c = hangeulToAlphabet[character - L'ㄱ'];
            if (isCapsLockOn)
            {
                if (std::isupper(c))
                {
                    result += static_cast<wchar_t>(std::tolower(c));
                }
                else
                {
                    result += static_cast<wchar_t>(std::toupper(c));
                }
            }
            else
            {
                result += c;
            }
        }
        else
        {
            result += character;
        }
    }

    return result;
}


std::wstring json5_error_to_string(const json5::error& err)
{
    std::wstring result;
    result.reserve(50);

    std::string errorType{ json5::error::type_string[err.type] };

    result += std::wstring{ errorType.begin(), errorType.end() };
    result += L"\n";

    result += L"Line: ";
    result += std::to_wstring(err.line);
    result += L", Column: ";
    result += std::to_wstring(err.column);

    return result;
}

