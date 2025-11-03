#pragma once
#include <string>

#include <doctest.h>

#include "Typoon/util/ToWString.h"
#include "Typoon/util/Unicode.h"


template<typoon::util::CanBeString T>
struct doctest::StringMaker<T>
{
    static String convert(const T& value)
    {
        const std::string s = typoon::util::to_u8_string(typoon::util::to_wstring(value));
        return String{ s.c_str() };
    }
};
