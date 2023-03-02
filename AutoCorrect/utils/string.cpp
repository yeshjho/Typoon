#include "./string.h"

#include <cwctype>

#include <uni-algo/conv.h>

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

std::wstring normalize_hangul(std::wstring_view str)
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

        if (L'가' <= c && c <= L'힣')
        {
            constexpr int lettersOfAChoseong = static_cast<int>(std::size(jungseongMap) * std::size(jongseongMap));
            auto [choseong, nonChoseong] = std::div(c - L'가', lettersOfAChoseong);
            auto [jungseong, jongseong] = std::div(nonChoseong, static_cast<int>(std::size(jongseongMap)));
            result += choseongMap[choseong];
            result += jungseongMap[jungseong];
            result += jongseongMap[jongseong];
        }
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
            result += jongseongMap[c - L'ᆨ' + 1];
        }
        else
        {
            result += c;
        }
    }

    return result;
}

