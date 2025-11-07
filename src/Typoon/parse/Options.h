#pragma once
#include <cstdint>


namespace typoon::parse
{

struct Options
{
    enum class EUppercaseStyle : std::uint8_t
    {
        first_letter,
        capitalize_words
    };

    bool case_sensitive = false;
    bool word = false;
    bool propagate_case = false;
    EUppercaseStyle uppercase_style = EUppercaseStyle::first_letter;
    bool full_composite = false;
    bool keep_composite = false;
    bool kor_eng_insensitive = false;

    Options& operator|=(const Options& other);
};

}
