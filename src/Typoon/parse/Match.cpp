#include "Match.h"

#include <ranges>
#include <type_traits>
#include <unordered_set>

#include "util/Assert.h"
#include "util/Hangeul.h"
#include "util/String.h"


namespace typoon::parse
{

std::vector<MatchValidateError> Match::Validate(const std::wstring_view cursorPlaceholder) const
{
    std::vector<MatchValidateError> errors;

    const bool isTriggerEmpty = trigger.empty();
    const bool isTriggersEmpty = triggers.empty();
    const auto triggersView =
        std::views::iota(size_t{ 0 }, triggers.size() + (isTriggerEmpty ? 0 : 1)) |
        std::views::transform(
            [this, isTriggerEmpty](const size_t index) -> const std::wstring&
            {
                if (isTriggerEmpty)
                {
                    return triggers.at(index);
                }
                else
                {
                    return index == 0 ? trigger : triggers.at(index - 1);
                }
            });

    const auto lambdaCheckTriggers =
        [&triggersView]<typename TFunc>(const TFunc& condition)
            requires std::is_invocable_r_v<bool, TFunc, const std::wstring&>
        {
            std::vector<size_t> erroneousTriggerIndices;
            size_t index = 0;
            for (const std::wstring& triggerToCheck : triggersView)
            {
                if (!condition(triggerToCheck))
                {
                    erroneousTriggerIndices.emplace_back(index);
                }
                index++;
            }

            return erroneousTriggerIndices;
        };

    const auto lambdaCheckTriggersAndReportError =
        [lambdaCheckTriggers, &errors]<typename TFunc>(const TFunc& condition, const EMatchValidateErrorType errorType)
            requires std::is_invocable_r_v<bool, TFunc, const std::wstring&>
        {
            if (std::vector<size_t> erroneousTriggerIndices = lambdaCheckTriggers(condition);
                !erroneousTriggerIndices.empty())
            {
                errors.emplace_back(MatchValidateError{ .type = errorType, .triggerIndices = std::move(erroneousTriggerIndices) });
            }
        };

    // any_of는 빈 경우 false를 반환하므로 empty 체크
    const std::vector<size_t> noCasedAlphabetTriggerIndices = 
        lambdaCheckTriggers([](const std::wstring& trigger) { return trigger.empty() || std::ranges::any_of(trigger, util::is_cased_alpha); });
    const auto lambdaCheckTriggersHaveCasedAlphabetAndReportError =
        [&noCasedAlphabetTriggerIndices, &errors](const EMatchValidateErrorType errorType)
        {
            if (!noCasedAlphabetTriggerIndices.empty())
            {
                errors.emplace_back(MatchValidateError{ .type = errorType, .triggerIndices = noCasedAlphabetTriggerIndices });
            }
        };

    const bool isReplaceEmpty = replace.empty();
    const bool isReplaceImageEmpty = replace_image.empty();
    const bool isReplaceCommandEmpty = replace_command.empty();

    const size_t cursorPlaceholderSize = cursorPlaceholder.size();


    // 트리거
    {
        if (isTriggerEmpty && isTriggersEmpty)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::NO_TRIGGER });
        }
        else
        {
            if (std::ranges::all_of(triggersView, [](const std::wstring& trigger) { return trigger.empty(); }))
            {
                errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::NO_TRIGGER });
            }

            lambdaCheckTriggersAndReportError(
                [](const std::wstring& trigger) { return !trigger.empty(); },
                EMatchValidateErrorType::EMPTY_TRIGGER
            );
        }

        if (!isTriggerEmpty && !isTriggersEmpty)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::TRIGGER_AND_TRIGGERS_BOTH_SET });
        }

        if (!isTriggerEmpty || !isTriggersEmpty)
        {
            std::unordered_set<std::wstring> seen;
            seen.reserve(triggersView.size());
            std::vector<size_t> duplicateTriggerIndices;
            size_t index = 0;
            for (const std::wstring& triggerToCheck : triggersView)
            {
                if (!seen.insert(triggerToCheck).second)
                {
                    duplicateTriggerIndices.emplace_back(index);
                }
                index++;
            }
            if (!duplicateTriggerIndices.empty())
            {
                errors.emplace_back(MatchValidateError{
                    .type = EMatchValidateErrorType::DUPLICATE_TRIGGERS, 
                    .triggerIndices = std::move(duplicateTriggerIndices)
                });
            }
        }
    }

    // 대치 텍스트
    {
        if (isReplaceEmpty && isReplaceImageEmpty && isReplaceCommandEmpty)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::NO_REPLACE });
        }
        else if (static_cast<int>(!isReplaceEmpty) + static_cast<int>(!isReplaceImageEmpty) + static_cast<int>(!isReplaceCommandEmpty)
                 > 1)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::MULTIPLE_REPLACE_SET });
        }

        if (!isReplaceEmpty)
        {
            if (const size_t pos = replace.find(cursorPlaceholder);
                pos != std::wstring::npos)
            {
                if (replace.find(cursorPlaceholder, pos + cursorPlaceholderSize) != std::wstring::npos)
                {
                    errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER });
                }
            }
        }
    }

    // case_sensitive
    if (case_sensitive)
    {
        lambdaCheckTriggersHaveCasedAlphabetAndReportError(EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET);

        if (kor_eng_insensitive)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::CASE_SENSITIVE_WITH_KOR_ENG_INSENSITIVE });
        }
    }

    // word
    if (word)
    {
        
    }

    // propagate_case
    if (propagate_case)
    {
        if (isReplaceEmpty)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::PROPAGATE_CASE_NON_TEXT_REPLACE });
        }

        if (case_sensitive)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::PROPAGATE_CASE_WITH_CASE_SENSITIVE });
        }

        lambdaCheckTriggersHaveCasedAlphabetAndReportError(EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET);
    }

    // uppercase_style

    // full_composite
    if (full_composite)
    {
        lambdaCheckTriggersAndReportError(
            [](const std::wstring& trigger) { return trigger.empty() || util::is_hangeul(trigger.back()); }, 
            EMatchValidateErrorType::FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL
        );

        if (word)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::FULL_COMPOSITE_WITH_WORD });
        }

        if (kor_eng_insensitive)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::FULL_COMPOSITE_WITH_KOR_ENG_INSENSITIVE });
        }
    }

    // keep_composite
    if (keep_composite)
    {
        if (isReplaceEmpty)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::KEEP_COMPOSITE_NON_TEXT_REPLACE });
        }

        if (!isReplaceEmpty && !util::is_hangeul(replace.back()))
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL });
        }

        if (full_composite)
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::KEEP_COMPOSITE_WITH_FULL_COMPOSITE });
        }

        if (const size_t index = replace.find(cursorPlaceholder);
            index != std::wstring::npos && index + cursorPlaceholderSize != replace.size())
        {
            errors.emplace_back(MatchValidateError{ .type = EMatchValidateErrorType::KEEP_COMPOSITE_CURSOR_NOT_AT_END });
        }

        if (!isReplaceEmpty)
        {
            const wchar_t replaceLastLetter = replace.back();
            lambdaCheckTriggersAndReportError(
                [replaceLastLetter](const std::wstring& trigger)
                {
                    return trigger.size() != 1 || trigger.back() != replaceLastLetter || !util::is_hangeul(trigger.back());
                },
                EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE
            );
        }
    }

    // kor_eng_insensitive
    if (kor_eng_insensitive)
    {
        lambdaCheckTriggersAndReportError(
            [](const std::wstring& trigger)
            {
                return std::ranges::any_of(trigger, [](const wchar_t c)
                {
                    return util::is_hangeul(c) || (L'A' <= c && c <= L'Z') || (L'a' <= c && c <= L'z');
                });
            },
            EMatchValidateErrorType::KOR_ENG_INSENSITIVE_NO_HANGEUL_OR_LATIN_ALPHABET
        );
    }

    return errors;
}

bool Match::TryFixUp(const std::wstring_view cursorPlaceholder)
{
    bool hasMetUnfixableError = false;

    // 일부 트리거만 문제가 돼도 해당 옵션을 비활성화함. 그런 경우 매치를 나누는 것이 맞기 때문.

    // triggers를 in-place 수정하기 때문에 수정 시 triggerIndices를 사용하면 안 됨

    for (const std::vector<MatchValidateError> errors = Validate(cursorPlaceholder);
        const MatchValidateError& error : errors)
    {
        switch (error.type)
        {
        case EMatchValidateErrorType::NO_TRIGGER:
            hasMetUnfixableError = true;
            break;

        case EMatchValidateErrorType::EMPTY_TRIGGER:
            std::erase_if(triggers, [](const std::wstring& trigger) { return trigger.empty(); });
            break;

        case EMatchValidateErrorType::TRIGGER_AND_TRIGGERS_BOTH_SET:
            triggers.insert(triggers.begin(), std::move(trigger));
            trigger.clear();
            break;

        case EMatchValidateErrorType::DUPLICATE_TRIGGERS:
        {
            std::unordered_set<std::wstring> seen;
            seen.reserve(triggers.size() + 1);
            seen.emplace(trigger);
            std::erase_if(triggers, [&seen](const std::wstring& trigger)
                {
                    return !seen.insert(trigger).second;
                });

            break;
        }

        case EMatchValidateErrorType::NO_REPLACE:
            hasMetUnfixableError = true;
            break;

        case EMatchValidateErrorType::MULTIPLE_REPLACE_SET:
            // 우선순위: replace > replace_image > replace_command
            if (!replace.empty())
            {
                replace_image.clear();
                replace_command.clear();
            }
            else if (!replace_image.empty())
            {
                replace_command.clear();
            }
            break;

        case EMatchValidateErrorType::MULTIPLE_CURSOR_PLACEHOLDER:
            {
                // 첫 번째만 남기기
                const size_t cursorPlaceholderSize = cursorPlaceholder.size();
                const size_t pos = replace.find(cursorPlaceholder) + cursorPlaceholderSize;
                while (true) 
                {
                    const size_t next = replace.find(cursorPlaceholder, pos);
                    if (next == std::string::npos)
                    {
                        break;
                    }
                    replace.erase(next, cursorPlaceholderSize);
                }
            }
            break;

        case EMatchValidateErrorType::CASE_SENSITIVE_NO_CASED_ALPHABET:
        case EMatchValidateErrorType::CASE_SENSITIVE_WITH_KOR_ENG_INSENSITIVE:  // kor_eng_insensitive 우선
            case_sensitive = false;  
            break;

        case EMatchValidateErrorType::PROPAGATE_CASE_NON_TEXT_REPLACE:
        case EMatchValidateErrorType::PROPAGATE_CASE_WITH_CASE_SENSITIVE:  // case_sensitive 우선
        case EMatchValidateErrorType::PROPAGATE_CASE_NO_CASED_ALPHABET:
            propagate_case = false;
            break;

        case EMatchValidateErrorType::FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL:
        case EMatchValidateErrorType::FULL_COMPOSITE_WITH_WORD:  // word 우선
        case EMatchValidateErrorType::FULL_COMPOSITE_WITH_KOR_ENG_INSENSITIVE:  // kor_eng_insensitive 우선
            full_composite = false;
            break;

        case EMatchValidateErrorType::KEEP_COMPOSITE_NON_TEXT_REPLACE:
        case EMatchValidateErrorType::KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL:
        case EMatchValidateErrorType::KEEP_COMPOSITE_WITH_FULL_COMPOSITE:  // full_composite 우선
        case EMatchValidateErrorType::KEEP_COMPOSITE_CURSOR_NOT_AT_END:
        case EMatchValidateErrorType::KEEP_COMPOSITE_RECURSIVE:
            keep_composite = false;
            break;

        case EMatchValidateErrorType::KOR_ENG_INSENSITIVE_NO_HANGEUL_OR_LATIN_ALPHABET:
            kor_eng_insensitive = false;
            break;

        default:
            ASSERT(false);
            std::unreachable();
        }

        if (hasMetUnfixableError)
        {
            break;
        }
    }

    return !hasMetUnfixableError;
}

}
