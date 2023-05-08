#include "../../common/common.h"

#include <ObjectArray.h>

#include "../../low_level/input_watcher.h"

#include "../../imm/imm_simulator.h"
#include "../../match/trigger_tree.h"
#include "../../utils/config.h"


bool is_on = false;

void turn_on(const std::any& data)
{
    if (is_on)
    {
        return;
    }

    start_input_watcher(std::any_cast<HWND>(data));
    setup_imm_simulator();
    setup_trigger_tree(get_config().matchFilePath);

    is_on = true;
}


void turn_off()
{
    if (!is_on)
    {
        return;
    }

    halt_trigger_tree_construction();
    end_input_watcher();
    teardown_trigger_tree();
    teardown_imm_simulator();

    is_on = false;
}


bool is_turned_on()
{
    return is_on;
}
