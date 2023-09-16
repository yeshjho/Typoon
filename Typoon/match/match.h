#pragma once
#include <string>
#include <vector>


struct Match
{
    enum class EUppercaseStyle
    {
        FIRST_LETTER,
        WORDS,
    };

    
    std::vector<std::wstring> triggers;
    std::wstring replace;
    std::filesystem::path replaceImage;
    bool isCaseSensitive;
    bool isWord;
    bool doPropagateCase;
    EUppercaseStyle uppercaseStyle;
    bool doNeedFullComposite;
    bool doKeepComposite;
};
