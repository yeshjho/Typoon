#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "Options.h"


namespace typoon::core
{

enum class EReplaceType : std::uint8_t
{
    TEXT,
    IMAGE,
    COMMAND,
};


/**
 * @brief typoon::parse::Match에서 validate가 끝난 매치
 */
struct Match
{
    std::vector<std::wstring> triggers;
    EReplaceType replaceType = EReplaceType::TEXT;
    std::wstring replace;

    core::Options options{};
};

}
