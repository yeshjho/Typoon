#pragma once
#include <string>
#include <string_view>
#include <type_traits>

#include <doctest.h>
#include <uni-algo/conv.h>


template<std::convertible_to<const std::wstring_view> T>
struct doctest::StringMaker<T>
{
    static String convert(const T& value)
    {
        const std::string s = una::utf16to8(value);
        return String{ s.c_str() };
    }
};


template<>
struct doctest::StringMaker<wchar_t>
{
    static String convert(const wchar_t& value)
    {
        const std::string s = una::utf16to8(std::wstring_view{ &value, 1 });
        return String{ s.c_str() };
    }
};
