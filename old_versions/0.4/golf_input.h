//
// Button
//

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
        app_log_error("button_set() too many ids trying to be assigned to button\n");

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

//
// Setup for game
//

enum Input_IDs {
// game
  IN_CARD_0,
  IN_CARD_1,
  IN_CARD_2,
  IN_CARD_3,

  IN_CARD_4,
  IN_CARD_5,
  IN_CARD_6,
  IN_CARD_7,

  IN_DRAW_PILE,
  IN_DISCARD_PILE,
  IN_PASS,

  IN_GAME_SIZE, // just as a test to see if the enum matches Game_Input

// menus
  IN_BACK,

#ifdef DEBUG

// camera
  IN_FORWARD,
  IN_BACKWARD,
  IN_LEFT,
  IN_RIGHT,
  IN_UP,
  IN_DOWN,


  IN_REFRESH_SHADERS,
  IN_TOGGLE_CAMERA,
  IN_RESET_GAME,
  IN_TOGGLE_WIREFRAME,

#endif // DEBUG

  IN_COUNT
};

//
// App
//

struct Mouse_Input {
  Vector2 coords;
  Vector2 relative_coords;
  Button left;
};

struct App_Input {
  Mouse_Input mouse;

  u32 last_input_type;

  Button buttons[IN_COUNT];
};

App_Input app_input = {};

enum App_Input_Types {
  IN_KEYBOARD,
  IN_MOUSE,
  IN_CONTROLLER,

  INPUT_TYPE_COUNT
};

inline void
set(u32 id, s32 sdl_id)  {
  Button *button = &app_input.buttons[id];
  if (button->num_of_ids > 2)
      app_log_error("button_set() too many ids trying to be assigned to button\n");

  button->ids[button->num_of_ids++] = sdl_id;
}

inline bool8
is_down(u32 id) {
  Button *button = &app_input.buttons[id];
  return is_down(*button);
}

inline bool8
on_down(u32 id) {
  Button *button = &app_input.buttons[id];
  return on_down(*button);
}

inline bool8
on_up(u32 id) {
  Button *button = &app_input.buttons[id];
  return on_up(*button);
}

internal void
init_buttons() {
  set(IN_BACK, SDLK_ESCAPE);

  set(IN_FORWARD, SDLK_W);
  set(IN_FORWARD, SDLK_UP);
  set(IN_BACKWARD, SDLK_S);
  set(IN_BACKWARD, SDLK_DOWN);
  set(IN_LEFT, SDLK_A);
  set(IN_LEFT, SDLK_LEFT);
  set(IN_RIGHT, SDLK_D);
  set(IN_RIGHT, SDLK_RIGHT);
  set(IN_UP, SDLK_SPACE);
  set(IN_DOWN, SDLK_LSHIFT);

  set(IN_REFRESH_SHADERS, SDLK_V);
  set(IN_TOGGLE_CAMERA, SDLK_C);
  set(IN_RESET_GAME, SDLK_K);
  set(IN_TOGGLE_WIREFRAME, SDLK_B);

  set(IN_CARD_0, SDLK_Q);
  set(IN_CARD_1, SDLK_W);
  set(IN_CARD_2, SDLK_E);
  set(IN_CARD_3, SDLK_R);

  set(IN_CARD_4, SDLK_A);
  set(IN_CARD_5, SDLK_S);
  set(IN_CARD_6, SDLK_D);
  set(IN_CARD_7, SDLK_F);

  set(IN_DRAW_PILE, SDLK_1);
  set(IN_DISCARD_PILE, SDLK_2);
  set(IN_PASS, SDLK_P);
}

inline void
app_input_set_previous_states() {
  app_input.mouse.relative_coords = {};
  app_input.mouse.left.previous_state = app_input.mouse.left.current_state;

  for (u32 i = 0; i < ARRAY_COUNT(app_input.buttons); i++) {
    Button *button = &app_input.buttons[i];
    button->previous_state = button->current_state;
  }
}
