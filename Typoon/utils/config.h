#pragma once
#include <filesystem>

#include "../low_level/hotkey.h"


struct ProgramOverride
{
    std::vector<std::wstring> programs;
    bool disable;
    std::filesystem::path matchFilePath;
    std::vector<std::filesystem::path> includes;
    std::vector<std::filesystem::path> excludes;

    bool operator==(const ProgramOverride& other) const = default;
};


struct RegexPreset
{
    std::wstring name;
    std::wstring expand;
};


struct Config
{
    std::filesystem::path matchFilePath;
    int maxBackspaceCount;
    std::wstring cursorPlaceholder;

    std::wstring regexCompositeStart;
    std::wstring regexCompositeEnd;
    std::wstring regexPresetSurround;
    std::vector<RegexPreset> regexPresets;

    bool notifyConfigLoad;
    bool notifyMatchLoad;
    bool notifyOnOff;

    std::pair<EKey, EModifierKey> toggleOnOffHotkey;
    std::pair<EKey, EModifierKey> getProgramNameHotkey;

    std::vector<ProgramOverride> programOverrides;
};


void read_config_file(const std::filesystem::path& filePath);

const Config& get_config();
