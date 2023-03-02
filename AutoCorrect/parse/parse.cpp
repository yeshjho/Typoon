#include "parse.h"

#include <json5/json5_input.hpp>
#include <json5/json5_reflect.hpp>

#include "../utils/logger.h"


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
        g_console_logger.Log(ELogLevel::ERROR, "Matches file", file.string(), "is invalid.");
    }
    return matches.matches;
}
