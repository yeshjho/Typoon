#include <doctest.h>

#include "Typoon/util/ToWString.h"

#include "util/DocTestWStringSupport.h"


TEST_SUITE("ToWString")
{
    TEST_CASE("to_wstring")
    {
        SUBCASE("char")
        {
            constexpr char c = 'A';
            const std::wstring expected{ L"A" };
            auto s = to_wstring(c);
            CHECK(to_wstring(c) == expected);
        }

        SUBCASE("wchar_t")
        {
            constexpr wchar_t c = L'가';
            const std::wstring expected{ L"가" };
            CHECK(to_wstring(c) == expected);
        }

        SUBCASE("char8_t")
        {
            constexpr char8_t c = u8'A';
            const std::wstring expected{ L"A" };
            CHECK(to_wstring(c) == expected);
        }

        SUBCASE("char[]")
        {
            constexpr char str[] = "Test String";
            const std::wstring expected{ L"Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("wchar_t[]")
        {
            constexpr wchar_t str[] = L"테스트 문자열Test String";
            const std::wstring expected{ L"테스트 문자열Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("char8_t[]")
        {
            constexpr char8_t str[] = u8"테스트 문자열Test String";
            const std::wstring expected{ L"테스트 문자열Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("const char*")
        {
            constexpr const char* str = "Test String";
            const std::wstring expected{ L"Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("const wchar_t*")
        {
            constexpr const wchar_t* str = L"테스트 문자열Test String";
            const std::wstring expected{ L"테스트 문자열Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("const char8_t*")
        {
            constexpr const char8_t* str = u8"테스트 문자열Test String";
            const std::wstring expected{ L"테스트 문자열Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("std::string")
        {
            const std::string str{ "Test String" };
            const std::wstring expected{ L"Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("std::wstring")
        {
            const std::wstring str{ L"테스트 문자열Test String" };
            const std::wstring expected{ L"테스트 문자열Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("std::u8string")
        {
            const std::u8string str{ u8"테스트 문자열Test String" };
            const std::wstring expected{ L"테스트 문자열Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("std::string_view")
        {
            constexpr std::string_view str = "Test String";
            const std::wstring expected{ L"Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("std::wstring_view")
        {
            constexpr std::wstring_view str = L"테스트 문자열Test String";
            const std::wstring expected{ L"테스트 문자열Test String" };
            CHECK(to_wstring(str) == expected);
        }

        SUBCASE("std::u8string_view")
        {
            constexpr std::u8string_view str = u8"테스트 문자열Test String";
            const std::wstring expected{ L"테스트 문자열Test String" };
            CHECK(to_wstring(str) == expected);
        }
    }
}
