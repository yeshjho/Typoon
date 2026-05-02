#pragma once
#include <string>
#include <string_view>
#include <type_traits>

#include "Unicode.h"


namespace typoon::util
{
    template<typename T>
    concept CanConstructWString = requires(T t)
    {
        std::wstring{ t };
    };

    template<typename T>
    concept CanCallToWString = requires(T t)
    {
        std::to_wstring(t);
    };

    template<typename T>
    concept CanConstructString = requires(T t)
    {
        std::string{ t };
    };

    template<typename T>
    concept CanCallToString = requires(T t)
    {
        std::to_string(t);
    };

    template<typename T>
    concept CanConstructU8String = requires(T t)
    {
        std::u8string{ t };
    };

    template<typename T>
    concept CanBeString = CanConstructWString<T> || CanCallToWString<T> || 
                          CanConstructString<T> || CanCallToString<T> ||
                          CanConstructU8String<T> ||
                          std::is_same_v<std::remove_cvref_t<T>, bool> ||
                          std::is_same_v<std::remove_cvref_t<T>, char> ||
                          std::is_same_v<std::remove_cvref_t<T>, wchar_t> ||
                          std::is_same_v<std::remove_cvref_t<T>, char8_t>;

    /**
     * @brief wstring, string을 construct할 수 있거나 to_wstring, to_string을 호출할 수 있는 타입을 wstring으로 변환
     */
    template<CanBeString T>
    std::wstring to_wstring(T&& t)
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<T>, bool>)
        {
            return t ? L"true" : L"false";
        }
        else if constexpr (CanConstructWString<T> || std::is_same_v<std::remove_cvref_t<T>, wchar_t>)
        {
            return std::wstring{ std::forward<T>(t) };
        }
        else if constexpr (CanConstructString<T> || std::is_same_v<std::remove_cvref_t<T>, char>)
        {
            return to_u16_string(std::string{ std::forward<T>(t) });
        }
        else if constexpr (CanConstructU8String<T> || std::is_same_v<std::remove_cvref_t<T>, char8_t>)
        {
            return to_u16_string(std::u8string{ std::forward<T>(t) });
        }
        else if constexpr (CanCallToWString<T>)
        {
            return std::to_wstring(std::forward<T>(t));
        }
        else if constexpr (CanCallToString<T>)
        {
            return to_u16_string(std::to_string(std::forward<T>(t)));
        }
        else
        {
            std::unreachable();
        }
    }
}
