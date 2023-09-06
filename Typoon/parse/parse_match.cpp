#include "parse_match.h"

#include <json5/json5_input.hpp>
#include <json5/json5_reflect.hpp>

#include "../low_level/tray_icon.h"
#include "../utils/logger.h"
#include "../utils/string.h"


OptionContainerForParse& OptionContainerForParse::operator|=(const OptionContainerForParse& other)
{
    case_sensitive |= other.case_sensitive;
    word |= other.word;
    propagate_case |= other.propagate_case;
    uppercase_style = other.uppercase_style == EUppercaseStyle::first_letter ? uppercase_style : other.uppercase_style;
    full_composite |= other.full_composite;
    keep_composite |= other.keep_composite;

    return *this;
}


MatchForParse::operator Match() const
{
    Match match{
        .replace = to_u16_string(replace),
        .isCaseSensitive = case_sensitive,
        .isWord = word,
        .doPropagateCase = propagate_case,
        .uppercaseStyle = static_cast<Match::EUppercaseStyle>(uppercase_style),
        .doNeedFullComposite = full_composite,
        .doKeepComposite = keep_composite,
    };

    if (!triggers.empty())
    {
        match.triggers.reserve(triggers.size());
        for (const std::string& trig : triggers)
        {
            match.triggers.emplace_back(to_u16_string(trig));
        }
    }
    else
    {
        match.triggers.emplace_back(to_u16_string(trigger));
    }
    return match;
}


std::vector<MatchForParse> parse_matches(const std::filesystem::path& file)
{
    if (std::ifstream ifs{ file }; ifs.is_open())
    {
        const std::string str{ std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
        return parse_matches(std::string_view{ str });
    }

    const std::wstring errorString = json5_error_to_string({ json5::error::could_not_open });
    logger.Log(ELogLevel::ERROR, "Matches file is invalid.", errorString);
    show_notification(L"Match File Parse Error", errorString);

    return {};
}


std::vector<MatchForParse> parse_matches(std::string_view matchesString)
{
    struct MatchesAndGroupsForParse
    {
        std::vector<GroupForParse> groups;
        std::vector<MatchForParse> matches;

        JSON5_MEMBERS(groups, matches)
    };

    json5::document doc;
    json5::error err;
    if (err = from_string(matchesString, doc);
        err == json5::error::none)
    {
        MatchesAndGroupsForParse matchesAndGroups;

        if (err = json5::from_document(doc, matchesAndGroups);
            err == json5::error::none)
        {
            size_t matchesLengthInGroups = 0;
            for (GroupForParse& group : matchesAndGroups.groups)
            {
                matchesLengthInGroups += group.matches.size();
            }
            matchesAndGroups.matches.reserve(matchesAndGroups.matches.size() + matchesLengthInGroups);

            for (GroupForParse& group : matchesAndGroups.groups)
            {
                for (MatchForParse& match : group.matches)
                {
                    match |= group;
                    matchesAndGroups.matches.emplace_back(std::move(match));
                }
            }

            return matchesAndGroups.matches;
        }
    }

    const std::wstring errorString = json5_error_to_string(err);
    logger.Log(ELogLevel::ERROR, "Matches string is invalid.", errorString);
    show_notification(L"Match File Parse Error", errorString);

    return {};
}
