#pragma once
#include <doctest.h>
#include <uni-algo/conv.h>

#include "../../Typoon/utils/config.h"


void simulate_type(std::wstring_view text);


inline Config default_config{ .maxBackspaceCount = 5, .cursorPlaceholder = L"|_|" };


namespace doctest
{
template<std::convertible_to<std::wstring_view> T>
struct StringMaker<T>
{
    static String convert(const T& value)
    {
        const std::string s = una::utf16to8(value);
        return String{ s.c_str() };
    }
};
}
