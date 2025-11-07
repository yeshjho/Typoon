#include <doctest.h>

#include "Typoon/parse/Match.h"

#include <concepts>
#include <ranges>


using typoon::parse::EMatchValidateErrorType;
using typoon::parse::Match;
using typoon::parse::MatchValidateError;

namespace
{

constexpr std::wstring_view CURSOR_PLACEHOLDER = L"|_|";

template<std::ranges::range T = std::initializer_list<MatchValidateError>>
    requires std::convertible_to<std::ranges::range_value_t<T>, MatchValidateError>
void check_errors(const Match& match, const T& expected)
{
    std::vector<MatchValidateError> errors = match.Validate(CURSOR_PLACEHOLDER);

    for (const MatchValidateError& error : expected)
    {
        const auto it = std::ranges::find(errors, error);
        CHECK(it != errors.end());
        errors.erase(it);
    }
}

template<std::ranges::range T = std::initializer_list<EMatchValidateErrorType>>
    requires std::convertible_to<std::ranges::range_value_t<T>, EMatchValidateErrorType>
void check_no_errors(const Match& match, const T& unexpected)
{
    std::vector<MatchValidateError> errors = match.Validate(CURSOR_PLACEHOLDER);

    for (const EMatchValidateErrorType& errorType : unexpected)
    {
        const auto it = 
            std::ranges::find_if(errors, [errorType](const MatchValidateError& error) { return error.type == errorType; });
        CHECK(it == errors.end());
    }
}

}

TEST_SUITE("Match")
{
    TEST_CASE("Validate")
    {
        SUBCASE("NO_TRIGGER - Positive")
        {
            {
                const Match match{
                    .trigger = L"",
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::NO_TRIGGER },
                });
            }
            {
                const Match match{
                    .triggers = {},
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::NO_TRIGGER },
                });
            }
            {
                const Match match{
                    .trigger = L"",
                    .triggers = {},
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::NO_TRIGGER },
                });
            }
            {
                const Match match{
                    .trigger = L"",
                    .triggers = { L"" },
                    .replace = L"#"
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::NO_TRIGGER },
                });
            }
            {
                const Match match{
                    .trigger = L"",
                    .triggers = { L"", L"" },
                    .replace = L"#"
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::NO_TRIGGER },
                });
            }
        }

        SUBCASE("NO_TRIGGER - Negative")
        {
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_TRIGGER,
                });
            }
            {
                const Match match{
                    .triggers = { L"@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_TRIGGER,
                });
            }
            {
                const Match match{
                    .triggers = { L"@", L"@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_TRIGGER,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { L"@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_TRIGGER,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { L"@", L"@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_TRIGGER,
                });
            }
        }

        SUBCASE("EMPTY_TRIGGER - Positive")
        {
            {
                const Match match{
                    .triggers = { L"" },
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::EMPTY_TRIGGER, .triggerIndices = { 0 } },
                });
            }
            {
                const Match match{
                    .triggers = { L"", L"@", L"" },
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::EMPTY_TRIGGER, .triggerIndices = { 0, 2 } },
                });
            }
        }

        SUBCASE("EMPTY_TRIGGER - Negative")
        {
            {
                const Match match{
                    .trigger = L"",
                    .replace = L"#"
                };
                check_no_errors(match, { 
                    EMatchValidateErrorType::EMPTY_TRIGGER,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                check_no_errors(match, { 
                    EMatchValidateErrorType::EMPTY_TRIGGER,
                });
            }
            {
                const Match match{
                    .triggers = { L"@" },
                    .replace = L"#"
                };
                check_no_errors(match, { 
                    EMatchValidateErrorType::EMPTY_TRIGGER,
                });
            }
            {
                const Match match{
                    .triggers = { L"@", L"@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::EMPTY_TRIGGER,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { L"@", L"@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::EMPTY_TRIGGER,
                });
            }
        }

        SUBCASE("TRIGGER_AND_TRIGGERS_BOTH_SET - Positive")
        {
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { L"@@" },
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::TRIGGER_AND_TRIGGERS_BOTH_SET },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { L"@@", L"@@@" },
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::TRIGGER_AND_TRIGGERS_BOTH_SET },
                });
            }
        }

        SUBCASE("TRIGGER_AND_TRIGGERS_BOTH_SET - Negative")
        {
            {
                const Match match{
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::TRIGGER_AND_TRIGGERS_BOTH_SET,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::TRIGGER_AND_TRIGGERS_BOTH_SET,
                });
            }
            {
                const Match match{
                    .trigger = L"",
                    .triggers = { L"@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::TRIGGER_AND_TRIGGERS_BOTH_SET,
                });
            }
            {
                const Match match{
                    .trigger = L"",
                    .triggers = { L"@", L"@@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::TRIGGER_AND_TRIGGERS_BOTH_SET,
                });
            }
        }

        SUBCASE("DUPLICATE_TRIGGERS - Positive")
        {
            {
                const Match match{
                    .triggers = { L"@", L"@" },
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::DUPLICATE_TRIGGERS, .triggerIndices = { 1 } },
                });
            }
            {
                const Match match{
                    .triggers = { L"@", L"@@", L"@" },
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::DUPLICATE_TRIGGERS, .triggerIndices = { 2 } },
                });
            }
            {
                const Match match{
                    .triggers = { L"@", L"@@", L"@", L"@@@", L"@" },
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::DUPLICATE_TRIGGERS, .triggerIndices = { 2, 4 } },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { L"@" },
                    .replace = L"#"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::DUPLICATE_TRIGGERS, .triggerIndices = { 1 } },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { L"@", L"@" },
                    .replace = L"#"
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::DUPLICATE_TRIGGERS, .triggerIndices = { 1, 2 } },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { L"@@", L"@" },
                    .replace = L"#"
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::DUPLICATE_TRIGGERS, .triggerIndices = { 2 } },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { L"@@", L"@@" },
                    .replace = L"#"
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::DUPLICATE_TRIGGERS, .triggerIndices = { 2 } },
                });
            }
        }

        SUBCASE("DUPLICATE_TRIGGERS - Negative")
        {
            {
                const Match match{
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::DUPLICATE_TRIGGERS,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::DUPLICATE_TRIGGERS,
                });
            }
            {
                const Match match{
                    .triggers = { L"@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::DUPLICATE_TRIGGERS,
                });
            }
            {
                const Match match{
                    .triggers = { L"@", L"@@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::DUPLICATE_TRIGGERS,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .triggers = { L"@@", L"@@@" },
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::DUPLICATE_TRIGGERS,
                });
            }
        }

        SUBCASE("NO_REPLACE - Positive")
        {
            {
                const Match match{
                    .trigger = L"@"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::NO_REPLACE },
                });
            }
        }

        SUBCASE("NO_REPLACE - Negative")
        {
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_REPLACE,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace_image = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_REPLACE,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace_command = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_REPLACE,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#",
                    .replace_image = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_REPLACE,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#",
                    .replace_command = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_REPLACE,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace_image = L"#",
                    .replace_command = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_REPLACE,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#",
                    .replace_image = L"#",
                    .replace_command = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::NO_REPLACE,
                });
            }
        }

        SUBCASE("MULTIPLE_REPLACE_SET - Positive")
        {
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#",
                    .replace_image = L"#"
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::MULTIPLE_REPLACE_SET },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#",
                    .replace_command = L"#"
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::MULTIPLE_REPLACE_SET },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace_image = L"#",
                    .replace_command = L"#"
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::MULTIPLE_REPLACE_SET },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#",
                    .replace_image = L"#",
                    .replace_command = L"#"
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::MULTIPLE_REPLACE_SET },
                });
            }
        }

        SUBCASE("MULTIPLE_REPLACE_SET - Negative")
        {
            {
                const Match match{
                    .trigger = L"@"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::MULTIPLE_REPLACE_SET,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::MULTIPLE_REPLACE_SET,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace_image = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::MULTIPLE_REPLACE_SET,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace_command = L"#"
                };
                check_no_errors(match, {
                    EMatchValidateErrorType::MULTIPLE_REPLACE_SET,
                });
            }
        }

#define CURSOR_PLACEHOLDER L"|_|"
        SUBCASE("MULTIPLE_CURSOR_PLACEHOLDER - Positive")
        {
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#" CURSOR_PLACEHOLDER L"#" CURSOR_PLACEHOLDER
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER L"#" CURSOR_PLACEHOLDER
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER L"###" CURSOR_PLACEHOLDER L"###"
                };
                check_errors(match, { 
                    { .type = EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER CURSOR_PLACEHOLDER
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER },
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER CURSOR_PLACEHOLDER CURSOR_PLACEHOLDER
                };
                check_errors(match, {
                    { .type = EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER },
                });
            }
        }

        SUBCASE("MULTIPLE_CURSOR_PLACEHOLDER - Negative")
        {
            {
                const Match match{
                    .trigger = L"@",
                };
                check_no_errors(match, { 
                    EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                check_no_errors(match, { 
                    EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = L"#" CURSOR_PLACEHOLDER
                };
                check_no_errors(match, { 
                    EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER L"#"
                };
                check_no_errors(match, { 
                    EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER,
                    .replace_image = CURSOR_PLACEHOLDER
                };
                check_no_errors(match, { 
                    EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER,
                    .replace_command = CURSOR_PLACEHOLDER
                };
                check_no_errors(match, { 
                    EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER,
                });
            }
            {
                const Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER L"__|_"
                };
                check_no_errors(match, { 
                    EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER,
                });
            }
        }
#undef CURSOR_PLACEHOLDER

        SUBCASE("CASE_SENSITIVE_NO_CASED_ALPHABET - Positive")
        {
            {
                Match match{
                    .trigger = L"한글",
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"123!@#,.;'",
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"こんにちは",
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"你好",
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"abc",
                    .triggers = { L"가나" },
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET, .triggerIndices = { 1 } },
                });
            }
            {
                Match match{
                    .trigger = L"가나",
                    .triggers = { L"abc", L"123" },
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET, .triggerIndices = { 0, 2 } },
                });
            }
            {
                Match match{
                    .triggers = { L"abc", L"123", L"가나" },
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET, .triggerIndices = { 1, 2 } },
                });
            }
        }

        SUBCASE("CASE_SENSITIVE_NO_CASED_ALPHABET - Negative")
        {
            {
                Match match{
                    .trigger = L"abc",
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_no_errors(match, { 
                    EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET,
                });
            }
            {
                Match match{
                    .trigger = L"ABC",
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_no_errors(match, { 
                    EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET,
                });
            }
            {
                Match match{
                    .trigger = L"가a.1",
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_no_errors(match, { 
                    EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET,
                });
            }
            {
                Match match{
                    .trigger = L"가a.1",
                    .triggers = { L"abc", L"123ABC" },
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_no_errors(match, { 
                    EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET,
                });
            }
        }

        SUBCASE("CASE_SENSITIVE_WITH_KOR_ENG_INSENSITIVE - Positive")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.case_sensitive = true;
                match.kor_eng_insensitive = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::CASE_SENSITIVE_WITH_KOR_ENG_INSENSITIVE },
                });
            }
        }

        SUBCASE("CASE_SENSITIVE_WITH_KOR_ENG_INSENSITIVE - Negative")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };

                check_no_errors(match, {
                    EMatchValidateErrorType::CASE_SENSITIVE_WITH_KOR_ENG_INSENSITIVE,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.case_sensitive = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::CASE_SENSITIVE_WITH_KOR_ENG_INSENSITIVE,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.kor_eng_insensitive = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::CASE_SENSITIVE_WITH_KOR_ENG_INSENSITIVE,
                });
            }
        }

        SUBCASE("PROPAGATE_CASE_NON_TEXT_REPLACE - Positive")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace_image = L"#"
                };
                match.propagate_case = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_NON_TEXT_REPLACE },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace_command = L"#"
                };
                match.propagate_case = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_NON_TEXT_REPLACE },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace_image = L"#",
                    .replace_command = L"#"
                };
                match.propagate_case = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_NON_TEXT_REPLACE },
                });
            }
        }

        SUBCASE("PROPAGATE_CASE_NON_TEXT_REPLACE - Negative")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::PROPAGATE_CASE_NON_TEXT_REPLACE,
                });
            }
        }

        SUBCASE("PROPAGATE_CASE_WITH_CASE_SENSITIVE - Positive")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace_image = L"#"
                };
                match.propagate_case = true;
                match.case_sensitive = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_WITH_CASE_SENSITIVE },
                });
            }
        }

        SUBCASE("PROPAGATE_CASE_WITH_CASE_SENSITIVE - Negative")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace_image = L"#"
                };

                check_no_errors(match, {
                    EMatchValidateErrorType::PROPAGATE_CASE_WITH_CASE_SENSITIVE,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace_image = L"#"
                };
                match.propagate_case = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::PROPAGATE_CASE_WITH_CASE_SENSITIVE,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace_image = L"#"
                };
                match.case_sensitive = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::PROPAGATE_CASE_WITH_CASE_SENSITIVE,
                });
            }
        }

        SUBCASE("PROPAGATE_CASE_NO_CASED_ALPHABET - Positive")
        {
            {
                Match match{
                    .trigger = L"한글",
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"123!@#,.;'",
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"こんにちは",
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"你好",
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"abc",
                    .triggers = { L"가나" },
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET, .triggerIndices = { 1 } },
                });
            }
            {
                Match match{
                    .trigger = L"가나",
                    .triggers = { L"abc", L"123" },
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET, .triggerIndices = { 0, 2 } },
                });
            }
            {
                Match match{
                    .triggers = { L"abc", L"123", L"가나" },
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET, .triggerIndices = { 1, 2 } },
                });
            }
        }

        SUBCASE("PROPAGATE_CASE_NO_CASED_ALPHABET - Negative")
        {
            {
                Match match{
                    .trigger = L"abc",
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_no_errors(match, { 
                    EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET,
                });
            }
            {
                Match match{
                    .trigger = L"ABC",
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_no_errors(match, { 
                    EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET,
                });
            }
            {
                Match match{
                    .trigger = L"가a.1",
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_no_errors(match, { 
                    EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET,
                });
            }
            {
                Match match{
                    .trigger = L"가a.1",
                    .triggers = { L"abc", L"123ABC" },
                    .replace = L"#"
                };
                match.propagate_case = true;

                check_no_errors(match, { 
                    EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET,
                });
            }
        }

        SUBCASE("FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL - Positive")
        {
            {
                Match match{
                    .triggers = { L"abc", L"123", L"가나" },
                    .replace = L"#"
                };
                match.full_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL, .triggerIndices = { 0, 1 } },
                });
            }
            {
                Match match{
                    .trigger = L"가나.",
                    .triggers = { L"abcㅏ", L"你好", L"가나" },
                    .replace = L"#"
                };
                match.full_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL, .triggerIndices = { 0, 2 } },
                });
            }
            {
                Match match{
                    .trigger = L"こんにちは",
                    .replace = L"#"
                };
                match.full_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL, .triggerIndices = { 0 } },
                });
            }
        }

        SUBCASE("FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL - Negative")
        {
            {
                Match match{
                    .trigger = L"abcㄺ",
                    .replace = L"#"
                };
                match.full_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
            {
                Match match{
                    .triggers = { L"...ㅟ", L"123ㄲ" },
                    .replace = L"#"
                };
                match.full_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
            {
                Match match{
                    .trigger = { L"가나" },
                    .triggers = { L"ㅑ", L"ㄱ" },
                    .replace = L"#"
                };
                match.full_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
            {
                Match match{
                    .trigger = { L"강낭콩" },
                    .replace = L"#"
                };
                match.full_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
        }

        SUBCASE("FULL_COMPOSITE_WITH_WORD - Positive")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.full_composite = true;
                match.word = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::FULL_COMPOSITE_WITH_WORD },
                });
            }
        }

        SUBCASE("FULL_COMPOSITE_WITH_WORD - Negative")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };

                check_no_errors(match, {
                    EMatchValidateErrorType::FULL_COMPOSITE_WITH_WORD,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.full_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::FULL_COMPOSITE_WITH_WORD,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.word = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::FULL_COMPOSITE_WITH_WORD,
                });
            }
        }

        SUBCASE("FULL_COMPOSITE_WITH_KOR_ENG_INSENSITIVE - Positive")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.full_composite = true;
                match.kor_eng_insensitive = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::FULL_COMPOSITE_WITH_KOR_ENG_INSENSITIVE },
                });
            }
        }

        SUBCASE("FULL_COMPOSITE_WITH_KOR_ENG_INSENSITIVE - Negative")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };

                check_no_errors(match, {
                    EMatchValidateErrorType::FULL_COMPOSITE_WITH_KOR_ENG_INSENSITIVE,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.full_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::FULL_COMPOSITE_WITH_KOR_ENG_INSENSITIVE,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.kor_eng_insensitive = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::FULL_COMPOSITE_WITH_KOR_ENG_INSENSITIVE,
                });
            }
        }

        SUBCASE("KEEP_COMPOSITE_NON_TEXT_REPLACE - Positive")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace_image = L"#"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_NON_TEXT_REPLACE },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace_command = L"#"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_NON_TEXT_REPLACE },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace_image = L"#",
                    .replace_command = L"#"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_NON_TEXT_REPLACE },
                });
            }
        }

        SUBCASE("KEEP_COMPOSITE_NON_TEXT_REPLACE - Negative")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_NON_TEXT_REPLACE,
                });
            }
        }

        SUBCASE("KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL - Positive")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"가나abc"
                };
                match.keep_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"가나123"
                };
                match.keep_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"가나."
                };
                match.keep_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"가나你好"
                };
                match.keep_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"가나こんにちは"
                };
                match.keep_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL },
                });
            }
        }

        SUBCASE("KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL - Negative")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"abcㄺ"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"...ㅟ"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"123ㄲ"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"가나"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"ㅑ"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"ㄱ"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"강낭콩"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL,
                });
            }
        }

        SUBCASE("KEEP_COMPOSITE_WITH_FULL_COMPOSITE - Positive")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.keep_composite = true;
                match.full_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_WITH_FULL_COMPOSITE },
                });
            }
        }

        SUBCASE("KEEP_COMPOSITE_WITH_FULL_COMPOSITE - Negative")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_WITH_FULL_COMPOSITE,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_WITH_FULL_COMPOSITE,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.full_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_WITH_FULL_COMPOSITE,
                });
            }
        }

#define CURSOR_PLACEHOLDER L"|_|"
        SUBCASE("KEEP_COMPOSITE_CURSOR_NOT_AT_END - Positive")
        {
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#" CURSOR_PLACEHOLDER L"#"
                };
                match.keep_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_CURSOR_NOT_AT_END },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER L"#"
                };
                match.keep_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_CURSOR_NOT_AT_END },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER L"###" CURSOR_PLACEHOLDER L"###"
                };
                match.keep_composite = true;

                check_errors(match, { 
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_CURSOR_NOT_AT_END },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER CURSOR_PLACEHOLDER
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_CURSOR_NOT_AT_END },
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = CURSOR_PLACEHOLDER CURSOR_PLACEHOLDER L"##"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_CURSOR_NOT_AT_END },
                });
            }
        }

        SUBCASE("KEEP_COMPOSITE_CURSOR_NOT_AT_END - Negative")
        {
            {
                Match match{
                    .trigger = L"@"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_CURSOR_NOT_AT_END,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_CURSOR_NOT_AT_END,
                });
            }
            {
                Match match{
                    .trigger = L"@",
                    .replace = L"#" CURSOR_PLACEHOLDER
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_CURSOR_NOT_AT_END,
                });
            }
        }
#undef CURSOR_PLACEHOLDER

        SUBCASE("KEEP_COMPOSITE_RECURSIVE - Positive")
        {
            {
                Match match{
                    .trigger = L"ㄱ",
                    .replace = L"ㄱ"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"ㅆ",
                    .replace = L"ㅆ"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"ㄺ",
                    .replace = L"ㄺ"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"ㅏ",
                    .triggers = { L"ㅏ" },
                    .replace = L"ㅏ"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE, .triggerIndices = { 0, 1 } },
                });
            }
            {
                Match match{
                    .trigger = L"ㅒ",
                    .replace = L"ㅒ"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"ㅙ",
                    .replace = L"ㅙ"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"가",
                    .replace = L"가"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"뛣",
                    .replace = L"뛣"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"뛣",
                    .replace = L"123뛣"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE, .triggerIndices = { 0 } },
                });
            }
            {
                Match match{
                    .trigger = L"뛣",
                    .replace = L"가뛣"
                };
                match.keep_composite = true;

                check_errors(match, {
                    { .type = EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE, .triggerIndices = { 0 } },
                });
            }
        }

        SUBCASE("KEEP_COMPOSITE_RECURSIVE - Negative")
        {
            {
                Match match{
                    .trigger = L"가"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE,
                });
            }
            {
                Match match{
                    .replace = L"가"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE,
                });
            }
            {
                Match match{
                    .trigger = L"가",
                    .replace = L"나"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE,
                });
            }
            {
                Match match{
                    .trigger = L"가",
                    .replace = L"나다"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE,
                });
            }
            {
                Match match{
                    .trigger = L"가뛣",
                    .replace = L"뛣"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE,
                });
            }
            {
                Match match{
                    .trigger = L"가뛣",
                    .replace = L"가뛣"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE,
                });
            }
            {
                Match match{
                    .trigger = L"a",
                    .replace = L"a"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE,
                });
            }
            {
                Match match{
                    .trigger = L".",
                    .replace = L"?."
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE,
                });
            }
            {
                Match match{
                    .trigger = L"1",
                    .replace = L"21"
                };
                match.keep_composite = true;

                check_no_errors(match, {
                    EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE,
                });
            }
        }
    }

    TEST_CASE("TryFixup")
    {
        SUBCASE("")
        {
            
        }
    }
}
