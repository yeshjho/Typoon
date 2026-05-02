#include "Unicode.h"

#include <uni-algo/conv.h>


namespace typoon::util
{
    std::wstring to_u16_string(const std::string_view str)
    {
        if (una::is_valid_utf8(str))
        {
            return una::utf8to16(str);
        }
        return {};
    }

    std::wstring to_u16_string(const std::u8string_view str)
    {
        if (una::is_valid_utf8(str))
        {
            return una::utf8to16(str);
        }
        return {};
    }

    std::string to_u8_string(const std::wstring_view str)
    {
        if (una::is_valid_utf16(str))
        {
            return una::utf16to8(str);
        }
        return {};
    }
}
