#include "trigger_trees_per_program.h"

#include <ranges>

#include "../utils/logger.h"


std::list<TriggerTree> trigger_trees;
std::unordered_map<std::wstring, TriggerTree*> trigger_tree_by_program;

std::wstring current_program;
TriggerTree* current_trigger_tree = nullptr;

std::filesystem::path default_match_file;


void setup_trigger_trees(const std::filesystem::path& defaultMatchFile)
{
    input_listeners.emplace_back("trigger_tree",
        [](const InputMessage(&inputs)[MAX_INPUT_COUNT], int length, bool clearAllAgents)
        {
            if (current_trigger_tree)
            {
                current_trigger_tree->OnInput(inputs, length, clearAllAgents);
            }
        });

    trigger_tree_by_program[DEFAULT_PROGRAM_NAME] = &trigger_trees.emplace_front(defaultMatchFile);
    default_match_file = defaultMatchFile;
}


void teardown_trigger_trees()
{
    std::erase_if(input_listeners, [](const std::pair<std::string, InputListener>& pair) { return pair.first == "trigger_tree"; });
    trigger_trees.clear();
    trigger_tree_by_program.clear();
    current_program.clear();
    current_trigger_tree = nullptr;
    default_match_file.clear();
}


TriggerTree* get_trigger_tree(const std::wstring& program)
{
    auto it = trigger_tree_by_program.find(program);
    if (it == trigger_tree_by_program.end())
    {
        it = trigger_tree_by_program.find(DEFAULT_PROGRAM_NAME);
    }

    return it->second;
}


void set_current_program(const std::wstring& program)
{
    if (current_program == program)
    {
        return;
    }

    logger.Log(ELogLevel::DEBUG, "Program changed to:", program);

    if (TriggerTree* next = get_trigger_tree(program);
        current_trigger_tree != next)
    {
        if (next)
        {
            next->ResetAgents();
        }
        current_trigger_tree = next;
    }

    current_program = program;
}


void reconstruct_all_trigger_trees(const std::function<void()>& onFinished)
{
    for (TriggerTree& triggerTree : trigger_trees | std::views::take(trigger_tree_by_program.size() - 1))
    {
        triggerTree.Reconstruct();
    }
    trigger_trees.back().Reconstruct({}, onFinished);
}


void update_trigger_tree_program_overrides(const std::vector<ProgramOverride>& programOverrides)
{
    auto it = trigger_trees.begin();
    ++it;
    trigger_trees.erase(it, trigger_trees.end());
    trigger_tree_by_program.clear();
    trigger_tree_by_program[DEFAULT_PROGRAM_NAME] = &trigger_trees.front();

    for (const auto& [programs, disable, matchFilePath, includes, excludes] : programOverrides)
    {
        TriggerTree* newTree = disable ? nullptr: &trigger_trees.emplace_back(matchFilePath.empty() ? default_match_file : matchFilePath, includes, excludes);
        for (const std::wstring& program : programs)
        {
            trigger_tree_by_program[program] = newTree;
        }
    }
}
