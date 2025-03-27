struct Button {    
    s32 ids[3];
    u32 num_of_ids;
    
    bool8 current_state;
    bool8 previous_state;
};

inline bool8
button_is_id(Button *button, s32 id) {
  for (u32 i = 0; i < button->num_of_ids; i++) {
    if (button->ids[i] == id) {
      return true;
    }
  }

  return false;
}

inline void
button_set(Button *button, s32 id)  {
    if (button->num_of_ids > 2)
        log_error("button_set() too many ids trying to be assigned to button\n");
    
    button->ids[button->num_of_ids++] = id;
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

struct App_Mouse_Input {
  Vector2 coords;
  Vector2 relative_coords;
  Button left;
};

struct App_Input {
  App_Mouse_Input mouse;

  union {
    struct {
      Button back;

      Button forward;
      Button backward;
      Button left;
      Button right;
      Button up;
      Button down;
    };
    Button buttons[7];
  };

  u32 last_input_type;
};

enum App_Input_Types {
  IN_KEYBOARD,
  IN_MOUSE,
  IN_CONTROLLER,

  INPUT_TYPE_COUNT
};
