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

