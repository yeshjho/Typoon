#include "parse_match.h"

#include "../low_level/tray_icon.h"
#include "../utils/json5_util.h"
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
        .replace = replace_image.empty() ? to_u16_string(replace) : std::wstring{},
        .replaceImage = replace_image,
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


std::pair<std::vector<MatchForParse>, std::set<std::filesystem::path>> parse_matches(const std::filesystem::path& file)
{
    std::set<std::filesystem::path> importedFiles;
    return { parse_matches(file, importedFiles), importedFiles };
}


std::vector<MatchForParse> parse_matches(const std::filesystem::path& file, std::set<std::filesystem::path>& importedFiles)
{
    const std::filesystem::path normalizedPath = file.lexically_normal();
    if (auto [_, wasNew] = importedFiles.insert(normalizedPath);
        !wasNew)
    {
        logger.Log(ELogLevel::WARNING, "Circular import detected:", file);
        return {};
    }

    std::wstring errorString;
    if (std::ifstream ifs{ file }; ifs.is_open())
    {
        const std::string str{ std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };

        json5::document doc;
        json5::error err;
        if (err = from_string(str, doc);
            err == json5::error::none)
        {
            struct Imports
            {
                std::vector<std::filesystem::path> imports;

                JSON5_MEMBERS(imports)
            } imports;

            if (err = json5::from_document(doc, imports);
                err == json5::error::none)
            {
                std::vector<MatchForParse> matches;

                for (const std::filesystem::path& importPath : imports.imports)
                {
                    std::vector<MatchForParse> importedMatches = parse_matches(importPath.is_absolute() ? importPath : (file.parent_path() / importPath), importedFiles);
                    std::ranges::move(importedMatches, std::back_inserter(matches));
                }

                std::vector<MatchForParse> mainMatches = parse_matches(std::string_view{ str });
                std::ranges::move(mainMatches, std::back_inserter(matches));

                return matches;
            }
        }

        errorString = json5_error_to_string(err);
    }

    if (errorString.empty())
    {
        errorString = json5_error_to_string({ json5::error::could_not_open });
    }
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
