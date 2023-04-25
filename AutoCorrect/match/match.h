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
    //bool isWord;  // TODO
    bool isCaseSensitive;
    bool doPropagateCase;
    //EUppercaseStyle uppercaseStyle;  // TODO
    bool doNeedFullComposite;
    bool doKeepComposite;
};
