#include "match_for_parse.h"

#include "../utils/string.h"


MatchForParse::operator Match() const
{
    Match match{
        .replace = to_u16_string(replace),
        .isCaseSensitive = case_sensitive,
        .doNeedFullComposite = full_composite,
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
