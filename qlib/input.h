enum Button_ID_Type {
    BUTTON_ID_TYPE_KEYBOARD,
    BUTTON_ID_TYPE_CONTROLLER,
    BUTTON_ID_TYPE_MOUSE
};

struct Button_ID {
    s32 id;
    u8 type;
};

struct Button {    
    Button_ID ids[3];
    u32 num_of_ids;

    u32 bitmaps[15]; // max amount of types of controllers (older sdl did not have a set way)
    
    bool8 current_state;
    bool8 previous_state;
};

inline Button_ID*
set(Button *button, s32 id)  {
    if (button->num_of_ids > 2)
        logprint("set()", "too many ids trying to be assigned to button\n");
    
    Button_ID *button_id = &button->ids[button->num_of_ids++];
    button_id->id = id;
    button_id->type = BUTTON_ID_TYPE_KEYBOARD;
    return button_id;
}

inline void
set_controller(Button *button, s32 id) {
    Button_ID *button_id = set(button, id);
    button_id->type = BUTTON_ID_TYPE_CONTROLLER;
}

inline bool8 
is_down(Button button) { 
    if (button.current_state) 
        return true;
    return false; 
}

inline bool8 
on_down(Button button) {
    if (button.current_state && button.current_state != button.previous_state) 
        return true;
    return false;
}

inline bool8
on_up(Button button) {
    if (!button.current_state && button.previous_state) 
        return true;
    else
        return false;
}

struct Input_Prompt {
    u32 id;
    const char *filename;
};

Input_Prompt xbox_prompts[] = {
    { SDL_CONTROLLER_BUTTON_A, "A" },
    { SDL_CONTROLLER_BUTTON_B, "B" },
    { SDL_CONTROLLER_BUTTON_X, "X" },
    { SDL_CONTROLLER_BUTTON_Y, "Y" },
    { SDL_CONTROLLER_BUTTON_DPAD_DOWN, "Dpad_Down" },
    { SDL_CONTROLLER_BUTTON_DPAD_LEFT, "Dpad_Left" },
    { SDL_CONTROLLER_BUTTON_DPAD_RIGHT, "Dpad_Right" },
    { SDL_CONTROLLER_BUTTON_DPAD_UP, "Dpad_Up" },
    { SDL_CONTROLLER_BUTTON_LEFTSHOULDER, "LB" },
    { SDL_CONTROLLER_BUTTON_LEFTSTICK, "Left_Stick_Click" },
};

// Matches sdl keycodes to files in the xelu prompt bitmap folder
Input_Prompt keyboard_prompts[] = {
    { SDLK_0, "0" },
    { SDLK_1, "1" },
    { SDLK_2, "2" },
    { SDLK_3, "3" },
    { SDLK_4, "4" },
    { SDLK_5, "5" },
    { SDLK_6, "6" },
    { SDLK_7, "7" },
    { SDLK_8, "8" },
    { SDLK_9, "9" },
    { SDLK_F1, "F1" },
    { SDLK_F2, "F2" },
    { SDLK_F3, "F3" },
    { SDLK_F4, "F4" },
    { SDLK_F5, "F5" },
    { SDLK_F6, "F6" },
    { SDLK_F7, "F7" },
    { SDLK_F8, "F8" },
    { SDLK_F9, "F9" },
    { SDLK_F10, "F10" },
    { SDLK_F11, "F11" },
    { SDLK_F12, "F12" },
    { SDLK_a, "A" },
    { SDLK_b, "B" },
    { SDLK_c, "C" },
    { SDLK_d, "D" },
    { SDLK_e, "E" },
    { SDLK_f, "F" },
    { SDLK_g, "G" },
    { SDLK_h, "H" },
    { SDLK_i, "I" },
    { SDLK_j, "J" },
    { SDLK_k, "K" },
    { SDLK_l, "L" },
    { SDLK_m, "M" },
    { SDLK_n, "N" },
    { SDLK_o, "O" },
    { SDLK_p, "P" },
    { SDLK_q, "Q" },
    { SDLK_r, "R" },
    { SDLK_s, "S" },
    { SDLK_t, "T" },
    { SDLK_u, "U" },
    { SDLK_v, "V" },
    { SDLK_w, "W" },
    { SDLK_x, "X" },
    { SDLK_y, "Y" },
    { SDLK_z, "Z" },
    { SDLK_LALT, "Alt" },
    { SDLK_DOWN, "Arrow_Down" },
    { SDLK_LEFT, "Arrow_Left" },
    { SDLK_RIGHT, "Arrow_Right" },
    { SDLK_UP, "Arrow_Up" },
    { SDLK_ASTERISK, "Asterisk" },
    { SDLK_BACKSPACE, "Backspace" },
    { SDLK_LEFTBRACKET, "Bracket_Left" },
    { SDLK_RIGHTBRACKET, "Bracket_Right" },
    { SDLK_CAPSLOCK, "Caps_Lock" },
    { SDLK_APPLICATION, "Command" },
    { SDLK_LCTRL, "Ctrl" },
    { SDLK_DELETE, "Del" },
    { SDLK_END, "End" },
    { SDLK_RETURN, "Enter" },
    { SDLK_ESCAPE, "Esc" },
    { SDLK_HOME, "Home" },
    { SDLK_INSERT, "Insert" },
    { SDLK_LESS, "Mark_Left" },
    { SDLK_GREATER, "Mark_Right" },
    { SDLK_MINUS, "Minus" },
    { SDLK_NUMLOCKCLEAR, "Num_Lock" },
    { SDLK_PAGEDOWN, "Page_Down" },
    { SDLK_PAGEUP, "Page_Up" },
    { SDLK_PLUS, "Plus" },
    { SDLK_PRINTSCREEN, "Print_Screen" },
    { SDLK_QUESTION, "Question" },
    { SDLK_QUOTEDBL, "Quote" },
    { SDLK_SEMICOLON, "Semicolon" },
    { SDLK_LSHIFT, "Shift" },
    { SDLK_SLASH, "Slash" },
    { SDLK_SPACE, "Space" },
    { SDLK_TAB, "Tab" },
    { SDLK_BACKQUOTE, "Tilda" },
    { SDLK_APPLICATION, "Win" },
};
