#pragma once
#include <filesystem>
#include <functional>
#include <set>


void reconstruct_trigger_tree_with(std::filesystem::path matchFile);
void setup_trigger_tree(std::filesystem::path matchFile);
void teardown_trigger_tree();
void reconstruct_trigger_tree(std::string_view matchesString = {}, std::function<void()> onFinish = {});
void halt_trigger_tree_construction();

inline std::set<std::filesystem::path> match_files_in_use;

// For unit tests
void wait_for_trigger_tree_construction();
