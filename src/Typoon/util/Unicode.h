#pragma once
#include <string>
#include <string_view>


namespace typoon::util
{

/**
 * @brief UTF-8 문자열을 UTF-16 문자열로 변환
 * @note 유효하지 않은 UTF-8 문자열이 들어올 경우 빈 문자열 반환
 */
std::wstring to_u16_string(std::string_view str);
std::wstring to_u16_string(std::u8string_view str);
/**
 * @brief UTF-16 문자열을 UTF-8 문자열로 변환
 * @note 유효하지 않은 UTF-16 문자열이 들어올 경우 빈 문자열 반환
 */
std::string to_u8_string(std::wstring_view str);

}
