void gfx_toggle_fullscreen(GFX_Window *window) {
	switch(window->display_mode) {
    case DISPLAY_MODE_WINDOWED: window->display_mode = DISPLAY_MODE_WINDOWED_FULLSCREEN; break;
    case DISPLAY_MODE_WINDOWED_FULLSCREEN:
    case DISPLAY_MODE_FULLSCREEN: window->display_mode = DISPLAY_MODE_WINDOWED; break;
	}
}