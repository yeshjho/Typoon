#include "parse_match.h"

#include <json5/json5_input.hpp>
#include <json5/json5_reflect.hpp>

#include "../low_level/tray_icon.h"
#include "../utils/logger.h"
#include "../utils/string.h"


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
    struct MatchesForParse
    {
        std::vector<MatchForParse> matches;

        JSON5_MEMBERS(matches)
    };
    MatchesForParse matches;

    json5::document doc;
    json5::error err;
    if (err = from_string(matchesString, doc);
        err == json5::error::none)
    {
        if (err = json5::from_document(doc, matches);
            err == json5::error::none)
        {
            return matches.matches;
        }
    }

    const std::wstring errorString = json5_error_to_string(err);
    logger.Log(ELogLevel::ERROR, "Matches string is invalid.", errorString);
    show_notification(L"Match File Parse Error", errorString);

    return matches.matches;
}
