#pragma once
#include <string>


// Whether the character is alphabetic and has a case.
bool is_cased_alpha(wchar_t c);

std::wstring to_u16_string(const std::string& str);
