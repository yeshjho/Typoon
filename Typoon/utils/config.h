#pragma once
#include <filesystem>


struct Config
{
    std::filesystem::path matchFilePath;
};


void read_config_file(const std::filesystem::path& filePath);

const Config& get_config();
