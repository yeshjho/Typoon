#pragma once
#include <string>

#include <doctest.h>

#include "Typoon/util/ToWString.h"
#include "Typoon/util/Unicode.h"


template<CanBeString T>
struct doctest::StringMaker<T>
{
    static String convert(const T& value)
    {
        const std::string s = to_u8_string(to_wstring(value));
        return String{ s.c_str() };
    }
};
