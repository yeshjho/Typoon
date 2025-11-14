#pragma once
#include <cstdint>


namespace typoon::core
{

enum class EUppercaseStyle : std::uint8_t
{
    FIRST_LETTER,
    CAPITALIZE_WORDS,
};

struct Options
{
    bool isCaseSensitive = false;
    bool isWord = false;
    bool doPropagateCase = false;
    EUppercaseStyle uppercaseStyle = EUppercaseStyle::FIRST_LETTER;
    bool isFullComposite = false;
    bool doKeepComposite = false;
    bool isKorEngInsensitive = false;
};

}
