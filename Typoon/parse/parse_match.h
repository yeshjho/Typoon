#pragma once
#include <filesystem>
#include <string>
#include <set>
#include <vector>

#include <json5/json5_base.hpp>

#include "../match/match.h"


// Helper classes for parsing the matches.
// If the names of the fields are identical to the strings in the data file, reflection part can be automatically handled by the json5 library.
struct OptionContainerForParse
{
    enum class EUppercaseStyle
    {
        first_letter,
        capitalize_words
    };

    bool case_sensitive = false;
    bool word = false;
    bool propagate_case = false;
    EUppercaseStyle uppercase_style = EUppercaseStyle::first_letter;
    bool full_composite = false;
    bool keep_composite = false;

    OptionContainerForParse& operator|=(const OptionContainerForParse& other);

    JSON5_MEMBERS(case_sensitive, word, propagate_case, uppercase_style, full_composite, keep_composite)
};

struct MatchForParse : OptionContainerForParse
{
    std::vector<std::string> triggers;
    std::string trigger;
    std::string replace;
    std::filesystem::path replace_image;
    std::string replace_command;

    JSON5_MEMBERS_INHERIT(OptionContainerForParse, triggers, trigger, replace, replace_image, replace_command)

    operator Match() const;
};

struct GroupForParse : OptionContainerForParse
{
    std::vector<MatchForParse> matches;

    JSON5_MEMBERS_INHERIT(OptionContainerForParse, matches)
};

JSON5_ENUM(OptionContainerForParse::EUppercaseStyle, first_letter, capitalize_words)


std::pair<std::vector<MatchForParse>, std::set<std::filesystem::path>> parse_matches(const std::filesystem::path& file,
    const std::vector<std::filesystem::path>& includes, const std::vector<std::filesystem::path>& excludes);
void invalidate_matches_cache(const std::filesystem::path& file);
void invalidate_all_matches_cache();
// For unit tests
std::vector<MatchForParse> parse_matches(std::string_view matchesString);
