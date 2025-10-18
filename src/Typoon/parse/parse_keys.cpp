#include "parse_keys.h"


EModifierKey get_combined_modifier(const HotKeyForParse& hotkey)
{
    EModifierKey modifier = EModifierKey::NONE;

    if (hotkey.ctrl)
    {
        modifier |= EModifierKey::CONTROL;
    }
    if (hotkey.shift)
    {
        modifier |= EModifierKey::SHIFT;
    }
    if (hotkey.alt)
    {
        modifier |= EModifierKey::ALT;
    }
    if (hotkey.win || hotkey.cmd)
    {
        modifier |= EModifierKey::SYSTEM;
    }

    return modifier;
}
