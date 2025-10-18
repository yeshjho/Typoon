#include "trigger_trees_per_program.h"

#include <ranges>

#include "../utils/logger.h"


std::list<TriggerTree> trigger_trees;
std::unordered_map<std::wstring, TriggerTree*> trigger_tree_by_program;

std::wstring current_program;
TriggerTree* current_trigger_tree = nullptr;

std::filesystem::path default_match_file;

std::atomic<std::atomic<size_t>*> latest_reconstruction_counter = nullptr;


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


template <typename T>
void reconstruct(T& treesToReconstruct, std::function<void()> onFinished)
{
    if (treesToReconstruct.empty())
    {
        return;
    }

    // Why are we allocating a new atomic every time?
    // Consider this: a reconstruction is triggered, affecting tree A, B, C.
    // Before the reconstruction is finished, another reconstruction is triggered, affecting tree A, B, D.
    // If we use the same atomic, the second reconstruction will be affected by the first one.
    // We can't blindly halt all construction either since tree C still needs to be reconstructed.
    auto* counter = new std::atomic<size_t>{ treesToReconstruct.size() };
    latest_reconstruction_counter = counter;

    const auto lambdaOnConstructionFinished = [onFinished = std::move(onFinished), counter]()
        {
            if (--(*counter) == 0)
            {
                if (onFinished && latest_reconstruction_counter == counter)
                {
                    onFinished();
                }

                delete counter;
            }
        };

    for (auto& triggerTree : treesToReconstruct)
    {
        if constexpr (std::is_pointer_v<std::remove_reference_t<decltype(triggerTree)>>)
        {
            triggerTree->Reconstruct({}, lambdaOnConstructionFinished);
        }
        else
        {
            triggerTree.Reconstruct({}, lambdaOnConstructionFinished);
        }
    }
}


void reconstruct_all_trigger_trees(std::function<void()> onFinished)
{
    reconstruct(trigger_trees, std::move(onFinished));
}


void reconstruct_trigger_trees_with_file(const std::filesystem::path& matchFile, std::function<void()> onFinished)
{
    reconstruct(trigger_trees_by_match_file[matchFile], std::move(onFinished));
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
