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
    std::wstring regexTrigger;
    std::wstring replace;
    std::filesystem::path replaceImage;
    std::wstring replaceCommand;
    bool isCaseSensitive;
    bool isWord;
    bool doPropagateCase;
    EUppercaseStyle uppercaseStyle;
    bool doNeedFullComposite;
    bool doKeepComposite;
    bool isKorEngInsensitive;
};
