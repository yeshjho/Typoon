#pragma once
#include "trigger_tree.h"

#include "../utils/config.h"


constexpr wchar_t DEFAULT_PROGRAM_NAME[]{ L"!!Default!!" };


void setup_trigger_trees(const std::filesystem::path& defaultMatchFile);
void teardown_trigger_trees();
TriggerTree* get_trigger_tree(const std::wstring& program);
void set_current_program(const std::wstring& program);
void reconstruct_all_trigger_trees(std::function<void()> onFinished = {});
void reconstruct_trigger_trees_with_file(const std::filesystem::path& matchFile, std::function<void()> onFinished = {});
void update_trigger_tree_program_overrides(const std::vector<ProgramOverride>& programOverrides);
