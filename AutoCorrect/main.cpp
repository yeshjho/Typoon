#include <iostream>

#include "low_level/keyboard_watcher.h"
#include "low_level/file_change_watcher.h"

#include "imm/ImmSimulator.h"
#include "input_multicast/input_multicast.h"
#include "match/trigger_tree.h"
#include "utils/logger.h"


int main()
{
    start_keyboard_watcher();
    start_file_change_watcher("C:/Users/yeshj/Desktop/folders/Visual Studio/AutoCorrect/AutoCorrect/data/", []()
                              {
                                  reconstruct_trigger_tree();
                              });
    setup_imm_simulator();
    setup_trigger_tree("C:/Users/yeshj/Desktop/folders/Visual Studio/AutoCorrect/AutoCorrect/data/test.json5");

    std::thread t{
        []()
        {
            auto& inputListener = register_input_listener();
            while (true)
            {
                const auto [letter, _] = inputListener.pop();
                g_console_logger.Log(letter);
            }
        }
    };

    char c;
    while (std::cin >> c)
    {
        
    }

    end_keyboard_watcher();
    end_file_change_watcher();
    teardown_imm_simulator();
    return 0;
}
