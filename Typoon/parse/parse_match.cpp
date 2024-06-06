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
        .regexTrigger = to_u16_string(regex_trigger),
        .replace = to_u16_string(replace),
        .replaceImage = replace_image,
        .replaceCommand = to_u16_string(replace_command),
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


std::unordered_map<std::filesystem::path, std::vector<MatchForParse>> matches_cache;


std::vector<MatchForParse> parse_matches(const std::filesystem::path& file, std::set<std::filesystem::path>& importedFiles,
    const std::vector<std::filesystem::path>& excludes)
{
    const std::filesystem::path normalizedPath = file.lexically_normal();
    if (auto [_, wasNew] = importedFiles.insert(normalizedPath);
        !wasNew)
    {
        logger.Log(ELogLevel::WARNING, "Circular import detected:", file);
        return {};
    }

    std::wstring errorString;
    if (std::ifstream ifs{ file })
    {
        const std::string str{ std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };

        json5::document doc;
        json5::error err;
        if (err = from_string(str, doc);
            err == json5::error::none)
        {
            std::vector<MatchForParse> matches;

            struct Imports
            {
                std::vector<std::filesystem::path> imports;

                JSON5_MEMBERS(imports)
            } imports;

            if (err = json5::from_document(doc, imports);
                err == json5::error::none)
            {
                for (const std::filesystem::path& importPath : imports.imports)
                {
                    const std::filesystem::path& absolutePath = importPath.is_absolute() ? importPath : (file.parent_path() / importPath);
                    if (std::ranges::find(excludes, absolutePath) != excludes.end())
                    {
                        continue;
                    }

                    std::vector<MatchForParse> importedMatches = parse_matches(absolutePath, importedFiles, excludes);
                    std::ranges::move(importedMatches, std::back_inserter(matches));
                }
            }

            if (const auto it = matches_cache.find(normalizedPath);
                it != matches_cache.end())
            {
                matches.insert_range(matches.end(), it->second);
            }
            else
            {
                std::vector<MatchForParse> mainMatches = parse_matches(std::string_view{ str });
                matches_cache[normalizedPath] = mainMatches;

                std::ranges::move(mainMatches, std::back_inserter(matches));
            }
            return matches;
        }

        errorString = json5_error_to_string(err);
    }

    if (errorString.empty())
    {
        char msg[256]{ 0, };
        strerror_s(msg, errno);
        errorString = { std::begin(msg), std::begin(msg) + std::strlen(msg) + 1 };
    }

    logger.Log(ELogLevel::ERROR, file, "Match file is invalid.", errorString);
    show_notification(L"Match File Parse Error", L"File: " + file.generic_wstring() + L"Error: " + errorString);

    return {};
}


std::pair<std::vector<MatchForParse>, std::set<std::filesystem::path>> parse_matches(const std::filesystem::path& file,
    const std::vector<std::filesystem::path>& includes, const std::vector<std::filesystem::path>& excludes)
{
    std::set<std::filesystem::path> importedFiles;
    std::vector<MatchForParse> matches = parse_matches(file, importedFiles, excludes);

    for (const std::filesystem::path& includePath : includes)
    {
        std::vector<MatchForParse> importedMatches = parse_matches(includePath.is_absolute() ? includePath : (file.parent_path() / includePath),
            importedFiles, excludes);
        std::ranges::move(importedMatches, std::back_inserter(matches));
    }

    return { std::move(matches), std::move(importedFiles) };
}


void invalidate_matches_cache(const std::filesystem::path& file)
{
    matches_cache.erase(file.lexically_normal());
}


void invalidate_all_matches_cache()
{
    matches_cache.clear();
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
