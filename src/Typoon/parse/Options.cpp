#include "Options.h"


namespace typoon::parse
{

Options& Options::operator|=(const Options& other)
{
    case_sensitive |= other.case_sensitive;
    word |= other.word;
    propagate_case |= other.propagate_case;
    uppercase_style = other.uppercase_style == EUppercaseStyle::first_letter ? uppercase_style : other.uppercase_style;
    full_composite |= other.full_composite;
    keep_composite |= other.keep_composite;
    kor_eng_insensitive |= other.kor_eng_insensitive;

    return *this;
}

}
