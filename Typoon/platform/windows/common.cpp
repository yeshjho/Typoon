#include "../../common/common.h"

#include <ObjectArray.h>

#include "../../low_level/clipboard.h"
#include "../../low_level/input_watcher.h"
#include "../../low_level/tray_icon.h"

#include "../../imm/imm_simulator.h"
#include "../../match/trigger_tree.h"
#include "../../utils/config.h"


bool is_on = false;

bool turn_on(const std::any& data)
{
    if (is_on)
    {
        return true;
    }

    if (!start_input_watcher(std::any_cast<HWND>(data)))
    {
        return false;
    }
    setup_imm_simulator();

    is_on = true;

    set_icon_on(true);
    if (get_config().notifyOnOff)
    {
        show_notification(L"Typoon", L"Typoon is on");
    }

    return true;
}


void turn_off()
{
    if (!is_on)
    {
        return;
    }

    halt_trigger_tree_construction();
    end_input_watcher();
    teardown_imm_simulator();
    pop_clipboard_state_without_restoring();

    is_on = false;

    set_icon_on(false);
    if (get_config().notifyOnOff)
    {
        show_notification(L"Typoon", L"Typoon is off");
    }
}


bool is_turned_on()
{
    return is_on;
}
