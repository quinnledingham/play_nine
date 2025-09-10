
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
  //gui->back_color = {0, 0, 0, 0.5};

  gui->shift = {0, 0.1f};
  gui->dim = {0.4f, 0.4f};
  gui->segments = {1, 4};
  gui->padding = {0.0f, 0.05f};
  gui->backdrop = {0.0f, 0.01f};

  s32 game_should_quit = false;
/*
  if (on_down(IN_BACK)) {
    print("YAHOO\n");
  }
*/
  Rect window_rect = {};
  window_rect.dim = cv2(vk_ctx.window_dim);

  const char *title = "Deck Nine";
  float32 title_pixel_height = vk_ctx.window_dim.y * 0.2f;
  Vector2 title_coords = centered_text_coords(window_rect, title, title_pixel_height, ALIGN_CENTER);

  gui_start(gui);

  title_coords.y -= vk_ctx.window_dim.y * 0.4f;
  title_coords += gui->backdrop_px;
  draw_text(title, title_coords, title_pixel_height, {0, 0, 0, 0.2f});
  title_coords -= gui->backdrop_px;
  draw_text(title, title_coords, title_pixel_height, play_nine_yellow);

  if (gui_button(gui, "Play", {0, 0})) {
    gui_manager.indices.pop();
    //game_draw.enabled = true;
    //sdl_set_relative_mouse_mode(true);
    //gui_manager.indices.push(GUI_DEBUG);
  }
  if (gui_button(gui, "Join", {0, 1})) {
    gui_manager.indices.push(GUI_TEST);
  }
  gui_button(gui, "Settings", {0, 2});
  if (gui_button(gui, "Quit", {0, 3}))
    game_should_quit = true;

  gui_end(gui);

  return game_should_quit;
}
/*
internal s32
draw_test_menu(GUI *gui) {
  if (on_down(IN_BACK)) {
    gui_manager.indices.pop();
  }

  Bitmap *bitmap = find_bitmap(BITMAP_LANA);
  draw_rect({300, 200}, cv2(bitmap->dim/5), bitmap);

  //draw_rect({400 ,400}, {100, 100}, &find_atlas(ATLAS_KEYBOARD)->bitmap);
  //u32 index = find_input_prompt_index(last_key, keyboard_prompts, ARRAY_COUNT(keyboard_prompts));
  //texture_atlas_draw_rect(ATLAS_KEYBOARD, index, {400 ,400}, {100, 100});

  draw_card_bitmaps();

  const char *lana = "lana";
  if (is_down(app_input.mouse.left)) {
    draw_text(lana, {300, 200}, 500, {200, 100, 0, 1});
  } else {    
    draw_text(lana, {700, 500}, 500, {100, 200, 0, 1});
  }

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

  if (on_down(IN_BACK)) {
    //sdl_toggle_relative_mouse_mode();
    gui_manager.indices.pop();
  }

  gui->background_color = play_nine_green;
  gui->background_color.a = 0.3f;

  gui->dim = {0.4f, 0.4f};
  gui->segments = {1, 2};
  gui->padding = {0.0f, 0.05f};
  gui->backdrop = {0.0f, 0.01f};

  gui_start(gui);

  gui_button(gui, "Settings", {0, 0});
  if (gui_button(gui, "Quit Game", {0, 1})) {
    gui_manager.indices.pop();
    gui_manager.indices.push(GUI_MAIN_MENU);
    game_draw.enabled = false;
  }

  gui_end(gui);

  return false;
}

internal s32
draw_debug_menu(GUI *gui) {
  gui->style.background_color = { 10, 10, 10, 0.5f };
  gui->style.background_color_hover = play_nine_light_yellow;
  gui->style.background_color_pressed = play_nine_dark_yellow;
  gui->style.background_color_active = play_nine_yellow;

  Vector4 white_color = { 255, 255, 255, 1 };
  gui->style.text_color = white_color;
  gui->style.text_color_hover = white_color;
  gui->style.text_color_pressed = white_color;
  gui->style.text_color_active = white_color;

  gui_start(gui);

  gui_textbox(gui, &camera.position.x, { 0, 0 });

  gui_end(gui);

  return false;
}
*/

internal void
init_guis() {
  gui_manager.guis[GUI_MAIN_MENU].draw = draw_main_menu;
  //gui_manager.guis[GUI_TEST].draw = draw_test_menu;
  //gui_manager.guis[GUI_PAUSE].draw = draw_pause_menu;
  //gui_manager.guis[GUI_DEBUG].draw = draw_debug_menu;

  gui_manager.indices.push(GUI_MAIN_MENU);
}
