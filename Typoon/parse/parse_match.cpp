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
    struct MatchesForParse
    {
        std::vector<MatchForParse> matches;

        JSON5_MEMBERS(matches)
    };
    MatchesForParse matches;
    if (const json5::error error = json5::from_file(file.string(), matches);
        error != json5::error::none)
    {
        const std::wstring errorString = json5_error_to_string(error);
        logger.Log(ELogLevel::ERROR, "Matches file", file.string(), "is invalid.", errorString);
        show_notification(L"Match File Parse Error", errorString);
    }
    return matches.matches;
}
