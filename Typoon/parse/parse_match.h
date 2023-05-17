#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include <json5/json5_base.hpp>

#include "../match/match.h"


// A helper class for parsing the matches.
// If the names of the fields are identical to the strings in the data file, reflection part can be automatically handled by the json5 library.
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
    bool case_sensitive = false;
    bool word = false;
    bool propagate_case = false;
    EUppercaseStyle uppercase_style = EUppercaseStyle::first_letter;
    bool full_composite = false;
    bool keep_composite = false;

    operator Match() const;

    JSON5_MEMBERS(triggers, trigger, replace, case_sensitive, word, propagate_case, uppercase_style, full_composite, keep_composite)
};

JSON5_ENUM(MatchForParse::EUppercaseStyle, first_letter, capitalize_words)


std::vector<MatchForParse> parse_matches(const std::filesystem::path& file);
