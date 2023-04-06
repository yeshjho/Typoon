#pragma once
#include <filesystem>


void initiate_trigger_tree(std::filesystem::path matchFile);
void reconstruct_trigger_tree();
void terminate_trigger_tree();
