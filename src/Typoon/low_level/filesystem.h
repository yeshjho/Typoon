#pragma once
#include <filesystem>


const std::filesystem::path& get_app_data_path();

const std::filesystem::path& get_config_file_path();

std::filesystem::path get_log_file_path();

std::filesystem::path get_crash_file_path();
