
internal s32
draw_main_menu(GUI *gui) {
  gui->style.background_color = play_nine_yellow;
  gui->style.background_color_hover = play_nine_light_yellow;
  gui->style.background_color_pressed = play_nine_dark_yellow;
  gui->style.background_color_active = play_nine_yellow;

  gui->style.text_color = play_nine_green;
  gui->style.text_color_hover = play_nine_green;
  gui->style.text_color_pressed = play_nine_green;
  gui->style.text_color_active = play_nine_green;

  gui->background_color = play_nine_green;

  gui->dim = {0.4, 0.4};
  gui->segments = {1, 4};

  s32 game_should_quit = false;

  if (on_down(app_input.back)) {
    print("YAHOO\n");
  }

  draw_gui(gui);

  if (gui_button(gui, "Play", {0, 0})) {
    gui_manager.indices.pop();
    draw_game_flag = true;
    sdl_set_relative_mouse_mode(true);
  }
  if (gui_button(gui, "Join", {0, 1})) {
    gui_manager.indices.push(GUI_TEST);
  }
  gui_button(gui, "Settings", {0, 2});
  if (gui_button(gui, "Quit", {0, 3}))
    game_should_quit = true;

  return game_should_quit;
}

internal s32
draw_test_menu(GUI *gui) {
  if (on_down(app_input.back)) {
    gui_manager.indices.pop();
  }

  Bitmap *bitmap = find_bitmap(BITMAP_LANA);
  draw_rect({300, 200}, cv2(bitmap->dim/5), bitmap);

  if (is_down(app_input.mouse.left))
    draw_text("LANA", {200, 100}, 100, {200, 100, 0, 1});
  else
    draw_text("LANA", {200, 100}, 100, {100, 200, 0, 1});

  //draw_rect({400 ,400}, {100, 100}, &find_atlas(ATLAS_KEYBOARD)->bitmap);
  //u32 index = find_input_prompt_index(last_key, keyboard_prompts, ARRAY_COUNT(keyboard_prompts));
  //texture_atlas_draw_rect(ATLAS_KEYBOARD, index, {400 ,400}, {100, 100});

  return false;
}

internal s32
draw_pause_menu(GUI *gui) {
  gui->style.background_color = play_nine_yellow;
  gui->style.background_color_hover = play_nine_light_yellow;
  gui->style.background_color_pressed = play_nine_dark_yellow;
  gui->style.background_color_active = play_nine_yellow;

  gui->style.text_color = play_nine_green;
  gui->style.text_color_hover = play_nine_green;
  gui->style.text_color_pressed = play_nine_green;
  gui->style.text_color_active = play_nine_green;

  if (on_down(app_input.back)) {
    sdl_toggle_relative_mouse_mode();
    gui_manager.indices.pop();
  }

  gui->dim = {0.4, 0.4};
  gui->segments = {1, 2};

  draw_gui(gui);

  gui_button(gui, "Settings", {0, 0});
  if (gui_button(gui, "Quit Game", {0, 1})) {
    gui_manager.indices.pop();
    gui_manager.indices.push(GUI_MAIN_MENU);
    draw_game_flag = false;
  }

  return false;
}

internal void
init_guis() {
  gui_manager.guis[GUI_MAIN_MENU].draw = draw_main_menu;
  gui_manager.guis[GUI_TEST].draw = draw_test_menu;
  gui_manager.guis[GUI_PAUSE].draw = draw_pause_menu;

  gui_manager.indices.push(GUI_MAIN_MENU);
}
