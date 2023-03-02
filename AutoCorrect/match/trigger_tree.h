#pragma once
#include <filesystem>

#include "match.h"


void initiate_trigger_tree(std::filesystem::path matchFile);
void reconstruct_trigger_tree();
void terminate_trigger_tree();
