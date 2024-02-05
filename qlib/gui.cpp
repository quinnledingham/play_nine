internal void
draw_button(const Draw_Button button) {
    Vector4 back_color = button.default_back_color;
    Vector4 text_color = button.default_text_color;
    
    if (button.active) {
        back_color = button.active_back_color;
        text_color = button.active_text_color;
    }

    float32 pixel_height = button.dim.y;
    if (button.dim.x < button.dim.y) pixel_height = button.dim.x;
    pixel_height *= 0.8f;

    Vector2 text_dim = get_string_dim(button.font, button.text, pixel_height, text_color);
    Vector2 text_coords = {};
    text_coords.x = button.coords.x + (button.dim.x / 2.0f) - (text_dim.x / 2.0f);
    text_coords.y = button.coords.y + (button.dim.y / 2.0f) + (text_dim.y / 2.0f);

    draw_rect(button.coords, 0, button.dim, back_color);                                           // back
    if (button.text) draw_string(button.font, button.text, text_coords, pixel_height, text_color); // text
}

// index  = number of button
// active = which index is active
// press  = interact key is pressed
internal bool8
menu_button(Menu *menu, const char *text, u32 index, u32 active, u32 press) {
    Draw_Button button = {
        menu->button_style.default_back_color,
        menu->button_style.default_text_color,

        menu->button_style.active_back_color,
        menu->button_style.active_text_color,

        false,

        menu->rect.coords,
        menu->button_style.dim,

        menu->font,
        text
    };

    bool8 button_pressed = false;
    if (index == active && press) button_pressed = true;
    if (index == active) button.active = true;

    draw_button(button);
    
    menu->rect.coords.y += menu->button_style.dim.y + menu->padding.y;
    
    return button_pressed;
}