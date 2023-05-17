#pragma once
#include "../low_level/hotkey.h"


struct HotKeyForParse
{
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    bool win = false;
    bool cmd = false;

    EKey key = EKey::INVALID;
};

