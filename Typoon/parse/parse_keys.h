#pragma once
#include "../low_level/hotkey.h"

#include <json5/json5_input.hpp>
#include <json5/json5_reflect.hpp>


struct HotKeyForParse
{
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    bool win = false;
    bool cmd = false;

    EKey key = EKey::INVALID;

    JSON5_MEMBERS(ctrl, shift, alt, win, cmd, key)
};

EModifierKey get_combined_modifier(const HotKeyForParse& hotkey);


constexpr std::string_view key_names[] = {
    "INVALID", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
    "space", "[", "]", "\\", ";", "'", ",", ".", "/", "tab", "caps_lock", "backspace", "enter", "`",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-", "=",
    "esc", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",
    "print_screen", "scroll_lock", "pause", "insert", "delete", "home", "end", "page_up", "page_down",
    "up", "down", "left", "right",
    "num_0", "num_1", "num_2", "num_3", "num_4", "num_5", "num_6", "num_7", "num_8", "num_9",
    "num_dot", "num_plus", "num_minus", "num_multiply", "num_divide", "num_enter", "num_lock",
};

namespace json5::detail
{

template<>
inline error read(const value& in, EKey& out)
{
    if (!in.is_string())
    {
        return { error::invalid_enum };
    }

    std::string inStr{ in.get_c_str() };
    std::ranges::transform(inStr, inStr.begin(), [](char c) { return std::tolower(c); });
    const std::string_view inStrView{ inStr };

    const auto it = std::ranges::find(key_names, inStrView);
    if (it == std::end(key_names))
    {
        return { error::invalid_enum };
    }

    out = static_cast<EKey>(std::distance(std::begin(key_names), it));
    return { error::none };
}

template<>
inline value write(writer& w, const HotKeyForParse& in)
{
    w.push_object();
    if (in.ctrl)
    {
        w[std::string_view{ "ctrl" }] = write(w, in.ctrl);
    }
    if (in.shift)
    {
        w[std::string_view{ "shift" }] = write(w, in.shift);
    }
    if (in.alt)
    {
        w[std::string_view{ "alt" }] = write(w, in.alt);
    }
    if (in.win)
    {
        w[std::string_view{ "win" }] = write(w, in.win);
    }
    if (in.cmd)
    {
        w[std::string_view{ "cmd" }] = write(w, in.cmd);
    }
    w[std::string_view{ "key" }] = write(w, std::string{ key_names[static_cast<int>(in.key)] });
    return w.pop();
}

}
