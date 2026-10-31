#pragma once
#include <Windows.h>
#include <array>


/*enum bindmode_t : char {
    ALWAYS = 0,
    HELD,
    HELDOFF,
    TOGGLE
};*/

class WndInput {
public:

    struct keystate_t {
        bool state;
        bool toggled_state;
    }; // size: 0x2

    std::array<keystate_t, 256> key_states;

    void setup(unsigned int& msg, WPARAM& wparam) {
        if (wparam > 256) // epic fail
            return;

        unsigned int vkey = wparam;

        bool is_msg_for_key = msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP;
        bool is_msg_for_mouse = msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP || msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP;

        if (!is_msg_for_key)
            return;

        bool is_key_held = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN || msg == WM_XBUTTONDOWN || msg == WM_MBUTTONDOWN;
        bool is_key_released = msg == WM_KEYUP || msg == WM_SYSKEYUP || msg == WM_XBUTTONUP || msg == WM_MBUTTONUP;

        keystate_t& current_key = key_states[vkey];

        current_key.state = is_key_held;

        // if we released the key, set the toggled state
        if (is_key_held)
            current_key.toggled_state = !current_key.toggled_state;
    }

    inline bool get_key_state(unsigned char key) {
        return key_states[key].state;
    }

    inline bool get_key_toggle_state(unsigned char key) {
        return key_states[key].toggled_state;
    }

    bool m_should_listen;
};

extern WndInput g_input;