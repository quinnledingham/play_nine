internal void
draw_main_menu() {
  GUI gui = {};
  gui.style.background_color = play_nine_yellow;
  gui.style.background_color_hover = play_nine_light_yellow;
  gui.style.background_color_pressed = play_nine_dark_yellow;
  gui.style.background_color_active = play_nine_yellow;

  gui.style.text_color = play_nine_green;
  gui.style.text_color_hover = play_nine_green;
  gui.style.text_color_pressed = play_nine_green;
  gui.style.text_color_active = play_nine_green;

  Rect rect = {};
  rect.coords = {100, 100};
  rect.dim = {100, 100};
  //draw_button(gui.style, 0, rect, "test", ALIGN_CENTER);

  gui.dim = {0.4, 0.4};
  gui.segments = {1, 5};

  draw_gui(&gui);

  gui_button(&gui, "aaaa", {0, 0});
  gui_button(&gui, "aaaaaaa", {0, 1});
  gui_button(&gui, "aaaaaaa", {0, 2});
  gui_button(&gui, "aaaaaaa", {0, 3});
  gui_button(&gui, "aaaaaaa", {0, 4});
}