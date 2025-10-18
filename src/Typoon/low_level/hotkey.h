#pragma once
#include <any>
#include <vector>


#undef DELETE

enum class [[nodiscard]] EKey
{
    INVALID = 0,

    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    SPACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    BACKSLASH,
    PIPE = BACKSLASH,
    SEMICOLON,
    COLON = SEMICOLON,
    QUOTE,
    COMMA,
    PERIOD,
    SLASH,
    QUESTION = SLASH,

    TAB,
    CAPS_LOCK,
    BACKSPACE,
    ENTER,

    TILDE,
    BACKTICK = TILDE,

    _0,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,

    DASH,
    UNDERSCORE = DASH,
    MINUS = DASH,
    EQUAL,
    PLUS = EQUAL,

    ESC,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    PRINT_SCREEN,
    SCROLL_LOCK,
    PAUSE_BREAK,

    INSERT,
    DELETE,
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,

    ARROW_UP,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,

    NUM_0,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,

    NUM_DOT,
    NUM_PLUS,
    NUM_MINUS,
    NUM_MULTIPLY,
    NUM_DIVIDE,
    NUM_ENTER,
    NUM_LOCK
};


enum class [[nodiscard]] EModifierKey : unsigned int
{
    NONE = 0,
    CONTROL = 1 << 0,
    SHIFT = 1 << 1,
    ALT = 1 << 2,
    SYSTEM = 1 << 3,
};


enum class [[nodiscard]] EHotKeyType
{
    TOGGLE_ON_OFF,
    GET_PROGRAM_NAME,
};


void start_hot_key_watcher(const std::any& data);

void reregister_hot_keys();

void end_hot_key_watcher();


constexpr EModifierKey operator|(EModifierKey lhs, EModifierKey rhs)
{
    return static_cast<EModifierKey>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}

constexpr EModifierKey& operator|=(EModifierKey& lhs, EModifierKey rhs)
{
    lhs = static_cast<EModifierKey>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
    return lhs;
}

constexpr EModifierKey operator&(EModifierKey lhs, EModifierKey rhs)
{
    return static_cast<EModifierKey>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
}
