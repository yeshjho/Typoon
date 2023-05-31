#pragma once
#include <filesystem>

#include "../low_level/hotkey.h"


struct Config
{
    std::filesystem::path matchFilePath;
    int maxBackspaceCount;
    std::wstring cursorPlaceholder;

    bool notifyConfigLoad;
    bool notifyMatchLoad;
    bool notifyOnOff;

    std::pair<EKey, EModifierKey> toggleOnOffHotkey;
};


void read_config_file(const std::filesystem::path& filePath);

const Config& get_config();
