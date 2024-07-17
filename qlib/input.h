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

    u32 bitmaps[SDL_CONTROLLER_TYPE_NVIDIA_SHIELD];
    
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

struct Entry {
    u32 key;
    char *value;
};

const char *prompt_folder = "../xelu/";

Entry sdl_prompt_xbox_bitmaps[1] = {
    { SDL_CONTROLLER_BUTTON_A, "Xbox Series/XboxSeriesX_A" }
};
