#include "parse.h"

#include <json5/json5_input.hpp>
#include <json5/json5_reflect.hpp>

#include "../low_level/tray_icon.h"
#include "../utils/logger.h"
#include "../utils/string.h"


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
