#pragma once
#include <filesystem>
#include <functional>


void start_file_change_watcher(std::filesystem::path path, std::function<void()> onChanged);
void end_file_change_watcher();
