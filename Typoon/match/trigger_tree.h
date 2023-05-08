#pragma once
#include <filesystem>


void setup_trigger_tree(std::filesystem::path matchFile);
void teardown_trigger_tree();
void reconstruct_trigger_tree();
void halt_trigger_tree_construction();
