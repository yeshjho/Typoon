#pragma once
#include <filesystem>


void setup_trigger_tree(std::filesystem::path matchFile);
void reconstruct_trigger_tree();
void teardown_trigger_tree();
