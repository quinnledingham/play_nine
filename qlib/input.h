struct Button {
    s32 id;
    
    s32 ids[3];
    u32 num_of_ids;
    
    bool8 current_state; 
    bool8 previous_state;    
};

inline void 
set(Button *button, s32 id)  {
    if (button->num_of_ids > 2)
        logprint("set()", "too many ids trying to be assigned to button\n");
    
    button->ids[button->num_of_ids++] = id;
    button->id = id; 
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
