enum Input_Prompt_Types {
    PROMPT_KEYBOARD,
    PROMPT_XBOX_SERIES,

    PROMPT_COUNT
};

struct Input_Prompt {
    u32 id;
    const char *filename;
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
    { SDLK_A, "A" },
    { SDLK_B, "B" },
    { SDLK_C, "C" },
    { SDLK_D, "D" },
    { SDLK_E, "E" },
    { SDLK_F, "F" },
    { SDLK_G, "G" },
    { SDLK_H, "H" },
    { SDLK_I, "I" },
    { SDLK_J, "J" },
    { SDLK_K, "K" },
    { SDLK_L, "L" },
    { SDLK_M, "M" },
    { SDLK_N, "N" },
    { SDLK_O, "O" },
    { SDLK_P, "P" },
    { SDLK_Q, "Q" },
    { SDLK_R, "R" },
    { SDLK_S, "S" },
    { SDLK_T, "T" },
    { SDLK_U, "U" },
    { SDLK_V, "V" },
    { SDLK_W, "W" },
    { SDLK_X, "X" },
    { SDLK_Y, "Y" },
    { SDLK_Z, "Z" },
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
    { SDLK_DBLAPOSTROPHE, "Quote" },
    { SDLK_SEMICOLON, "Semicolon" },
    { SDLK_LSHIFT, "Shift" },
    { SDLK_SLASH, "Slash" },
    { SDLK_SPACE, "Space" },
    { SDLK_TAB, "Tab" },
    { SDLK_GRAVE, "Tilda" },
    { SDLK_APPLICATION, "Win" },
};

internal void
create_input_prompt_atlas(Texture_Atlas *atlas, Input_Prompt *prompts, u32 num_of_prompts, const char *folder_path, const char *key_type, const char *filename) {
  
  *atlas = create_texture_atlas(1500, 1000, 4);
  
  // dynamically creating filenames to load a bunch of separate files
  u32 folder_path_length = str_length(folder_path);
  char filepath[100];
  memset(filepath, 0, 100);
  memcpy(filepath, folder_path, folder_path_length);

  for (u32 prompt_index = 0; prompt_index < num_of_prompts; prompt_index++) {
    char *path = filepath + folder_path_length;
    memset(path, 0, 100 - folder_path_length);
    
    const char *filename = prompts[prompt_index].filename;
    u32 filename_length = str_length(filename);
    memcpy(path, filename, filename_length);
    path += filename_length;
    memcpy(path, key_type, str_length(key_type));

    File file = load_file(filepath);
    Bitmap prompt_bitmap = load_bitmap(file);

    texture_atlas_add(atlas, &prompt_bitmap);
  }  
}

internal u32
find_input_prompt_index(s32 id, Input_Prompt *prompts, u32 num_of_prompts) {
  for (u32 prompt_index = 0; prompt_index < num_of_prompts; prompt_index++) {
    if (prompts[prompt_index].id == id)
      return prompt_index;
  }

  log_error("find_input_prompt_index() did not find prompt index\n");
  return 0;
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