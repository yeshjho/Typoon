#pragma once
#include <filesystem>

#include "match_for_parse.h"


std::vector<MatchForParse> parse_matches(const std::filesystem::path& file);
