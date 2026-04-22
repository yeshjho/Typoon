#pragma once
#include <cstdint>

#include <vector>


namespace typoon::parse
{

enum class EMatchValidateErrorType : std::uint8_t
{
    /// 트리거
    NO_TRIGGER,
    EMPTY_TRIGGER,
    TRIGGER_AND_TRIGGERS_BOTH_SET,
    DUPLICATE_TRIGGERS,

    /// 대치 텍스트
    NO_REPLACE,
    MULTIPLE_REPLACE_SET,
    MULTIPLE_CURSOR_PLACEHOLDER,

    /// case_sensitive
    CASE_SENSITIVE_NO_CASED_ALPHABET,
    CASE_SENSITIVE_WITH_KOR_ENG_INSENSITIVE,

    /// word

    /// propagate_case
    PROPAGATE_CASE_NON_TEXT_REPLACE,
    PROPAGATE_CASE_WITH_CASE_SENSITIVE,
    PROPAGATE_CASE_NO_CASED_ALPHABET,

    /// uppercase_style

    /// full_composite
    FULL_COMPOSITE_LAST_LETTER_NON_HANGEUL,
    FULL_COMPOSITE_WITH_WORD,
    FULL_COMPOSITE_WITH_KOR_ENG_INSENSITIVE,

    /// keep_composite
    KEEP_COMPOSITE_NON_TEXT_REPLACE,
    KEEP_COMPOSITE_LAST_LETTER_NON_HANGEUL,
    KEEP_COMPOSITE_WITH_FULL_COMPOSITE,
    KEEP_COMPOSITE_CURSOR_NOT_AT_END,
    KEEP_COMPOSITE_RECURSIVE,
    KEEP_COMPOSITE_WITH_PASTE_TO_REPLACE,

    /// kor_eng_insensitive
    KOR_ENG_INSENSITIVE_NO_HANGEUL_OR_LATIN_ALPHABET,

    /// paste_to_replace
};

struct MatchValidateError
{
    EMatchValidateErrorType type;
    /**
     * @brief 트리거 관련 에러인 경우, 문제가 되는 트리거들의 인덱스
     * @note trigger와 triggers가 둘 다 설정된 경우 trigger를 triggers의 맨 앞에 추가함에 주의.
     */
    std::vector<size_t> triggerIndices{};

    bool operator==(const MatchValidateError&) const = default;
};

}
