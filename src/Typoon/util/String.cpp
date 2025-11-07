#include "String.h"

#include <cwctype>


namespace typoon::util
{

bool is_cased_alpha(const wchar_t ch)
{
    return std::iswalpha(ch) && (std::iswupper(ch) ^ std::iswlower(ch));
}

}
