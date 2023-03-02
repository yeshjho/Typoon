#pragma once
#include <string>
#include <vector>

#include <json5/json5_base.hpp>

#include "../match/match.h"


struct MatchForParse
{
    enum class EUppercaseStyle
    {
        first_letter,
        capitalize_words
    };

    std::vector<std::string> triggers;
    std::string trigger;
    std::string replace;
    bool word = false;
    bool case_sensitive = false;
    bool propagate_case = false;
    EUppercaseStyle uppercase_style = EUppercaseStyle::first_letter;

    explicit operator Match() const;

    JSON5_MEMBERS(triggers, trigger, replace, word, case_sensitive, propagate_case, uppercase_style)
};

JSON5_ENUM(MatchForParse::EUppercaseStyle, first_letter, capitalize_words)
