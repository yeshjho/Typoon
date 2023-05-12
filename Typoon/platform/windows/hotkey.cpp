#include "../../low_level/hotkey.h"

#include <Windows.h>
#undef DELETE

#include <algorithm>
#include <iterator>

#include "../../common/common.h"
#include "../../utils/logger.h"
#include "wnd_proc.h"


int get_vkey_code(EKey key)
{
    switch (key)
    {
    case EKey::A:
        return 'A';
    case EKey::B:
        return 'B';
    case EKey::C:
        return 'C';
    case EKey::D:
        return 'D';
    case EKey::E:
        return 'E';
    case EKey::F:
        return 'F';
    case EKey::G:
        return 'G';
    case EKey::H:
        return 'H';
    case EKey::I:
        return 'I';
    case EKey::J:
        return 'J';
    case EKey::K:
        return 'K';
    case EKey::L:
        return 'L';
    case EKey::M:
        return 'M';
    case EKey::N:
        return 'N';
    case EKey::O:
        return 'O';
    case EKey::P:
        return 'P';
    case EKey::Q:
        return 'Q';
    case EKey::R:
        return 'R';
    case EKey::S:
        return 'S';
    case EKey::T:
        return 'T';
    case EKey::U:
        return 'U';
    case EKey::V:
        return 'V';
    case EKey::W:
        return 'W';
    case EKey::X:
        return 'X';
    case EKey::Y:
        return 'Y';
    case EKey::Z:
        return 'Z';
    case EKey::SPACE:
        return VK_SPACE;
    case EKey::LEFT_BRACKET:
        return VK_OEM_4;
    case EKey::RIGHT_BRACKET:
        return VK_OEM_6;
    case EKey::BACKSLASH:
        return VK_OEM_5;
    case EKey::SEMICOLON:
        return VK_OEM_1;
    case EKey::QUOTE:
        return VK_OEM_7;
    case EKey::COMMA:
        return VK_OEM_COMMA;
    case EKey::PERIOD:
        return VK_OEM_PERIOD;
    case EKey::SLASH:
        return VK_OEM_2;
    case EKey::TAB:
        return VK_TAB;
    case EKey::CAPS_LOCK:
        return VK_CAPITAL;
    case EKey::BACKSPACE:
        return VK_BACK;
    case EKey::ENTER:
        return VK_RETURN;
    case EKey::TILDE:
        return VK_OEM_3;
    case EKey::_0:
        return '0';
    case EKey::_1:
        return '1';
    case EKey::_2:
        return '2';
    case EKey::_3:
        return '3';
    case EKey::_4:
        return '4';
    case EKey::_5:
        return '5';
    case EKey::_6:
        return '6';
    case EKey::_7:
        return '7';
    case EKey::_8:
        return '8';
    case EKey::_9:
        return '9';
    case EKey::DASH:
        return VK_OEM_MINUS;
    case EKey::EQUAL:
        return VK_OEM_PLUS;
    case EKey::ESC:
        return VK_ESCAPE;
    case EKey::F1:
        return VK_F1;
    case EKey::F2:
        return VK_F2;
    case EKey::F3:
        return VK_F3;
    case EKey::F4:
        return VK_F4;
    case EKey::F5:
        return VK_F5;
    case EKey::F6:
        return VK_F6;
    case EKey::F7:
        return VK_F7;
    case EKey::F8:
        return VK_F8;
    case EKey::F9:
        return VK_F9;
    case EKey::F10:
        return VK_F10;
    case EKey::F11:
        return VK_F11;
    case EKey::F12:
        return VK_F12;
    case EKey::PRINT_SCREEN:
        return VK_SNAPSHOT;
    case EKey::SCROLL_LOCK:
        return VK_SCROLL;
    case EKey::PAUSE_BREAK:
        return VK_PAUSE;
    case EKey::INSERT:
        return VK_INSERT;
    case EKey::DELETE:
        return VK_DELETE;
    case EKey::HOME:
        return VK_HOME;
    case EKey::END:
        return VK_END;
    case EKey::PAGE_UP:
        return VK_PRIOR;
    case EKey::PAGE_DOWN:
        return VK_NEXT;
    case EKey::ARROW_UP:
        return VK_UP;
    case EKey::ARROW_DOWN:
        return VK_DOWN;
    case EKey::ARROW_LEFT:
        return VK_LEFT;
    case EKey::ARROW_RIGHT:
        return VK_RIGHT;
    case EKey::NUM_0:
        return VK_NUMPAD0;
    case EKey::NUM_1:
        return VK_NUMPAD1;
    case EKey::NUM_2:
        return VK_NUMPAD2;
    case EKey::NUM_3:
        return VK_NUMPAD3;
    case EKey::NUM_4:
        return VK_NUMPAD4;
    case EKey::NUM_5:
        return VK_NUMPAD5;
    case EKey::NUM_6:
        return VK_NUMPAD6;
    case EKey::NUM_7:
        return VK_NUMPAD7;
    case EKey::NUM_8:
        return VK_NUMPAD8;
    case EKey::NUM_9:
        return VK_NUMPAD9;
    case EKey::NUM_DOT:
        return VK_DECIMAL;
    case EKey::NUM_PLUS:
        return VK_ADD;
    case EKey::NUM_MINUS:
        return VK_SUBTRACT;
    case EKey::NUM_MULTIPLY:
        return VK_MULTIPLY;
    case EKey::NUM_DIVIDE:
        return VK_DIVIDE;
    case EKey::NUM_ENTER:
        return VK_RETURN;  // Windows doesn't distinguish between numpad enter and regular enter
    case EKey::NUM_LOCK:
        return VK_NUMLOCK;
    default:
        throw;
    }
}


unsigned int get_modifier_code(EModifierKey modifierKey)
{
    unsigned int code = 0;

    if (static_cast<bool>(modifierKey & EModifierKey::CONTROL))
    {
        code |= MOD_CONTROL;
    }
    if (static_cast<bool>(modifierKey & EModifierKey::SHIFT))
    {
        code |= MOD_SHIFT;
    }
    if (static_cast<bool>(modifierKey & EModifierKey::ALT))
    {
        code |= MOD_ALT;
    }
    if (static_cast<bool>(modifierKey & EModifierKey::SYSTEM))
    {
        code |= MOD_WIN;
    }

    return code;
}


HWND hot_key_hwnd;
std::vector<std::tuple<EHotKeyType, int, int>> hot_keys;


void start_hot_key_watcher(const std::vector<std::tuple<EHotKeyType, EKey, EModifierKey>>& hotKeys, const std::any& data)
{
    hot_key_hwnd = std::any_cast<HWND>(data);
    std::ranges::transform(hotKeys, std::back_inserter(hot_keys), [](const auto& tuple)
        {
            return std::make_tuple(std::get<0>(tuple), get_vkey_code(std::get<1>(tuple)), get_modifier_code(std::get<2>(tuple)));
        });

    for (const auto& [type, key, modifiers] : hotKeys)
    {
        if (!RegisterHotKey(hot_key_hwnd, 0, get_modifier_code(modifiers), get_vkey_code(key)))
        {
            logger.Log(ELogLevel::ERROR, "RegisterHotKey failed:", std::system_category().message(static_cast<int>(GetLastError())));
        }
    }

    wnd_proc_functions.emplace_back("hot_key", 
        []([[maybe_unused]] HWND hWnd, UINT msg, [[maybe_unused]] WPARAM wParam, LPARAM lParam) -> std::optional<LRESULT>
        {
            if (msg != WM_HOTKEY)
            {
                return std::nullopt;
            }

            const int modifiers = LOWORD(lParam);
            const int key = HIWORD(lParam);

            const auto it = std::ranges::find_if(hot_keys, [key, modifiers](const auto& tuple)
                {
                    return std::get<1>(tuple) == key && std::get<2>(tuple) == modifiers;
                });
            if (it == hot_keys.end())
            {
                return 0;
            }

            switch (std::get<0>(*it))
            {
            case EHotKeyType::TOGGLE_ON_OFF:
                is_turned_on() ? turn_off() : turn_on(hot_key_hwnd);
                break;

            default:
                throw;
            }

            return 0;
        });
}


void end_hot_key_watcher()
{
    std::erase_if(wnd_proc_functions, [](const std::pair<std::string, WndProcFunc>& pair) { return pair.first == "hot_key"; });

    while (UnregisterHotKey(hot_key_hwnd, 0))
    {}

    hot_keys.clear();
}
