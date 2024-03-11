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
    if (index == active && press)
        button_pressed = true;
    if (index == active)
        button.active = true;

    draw_button(button);
    
    menu->rect.coords.y += menu->button_style.dim.y + menu->padding.y;
    
    return button_pressed;
}

internal bool8
menu_in_dim(Vector2_s32 coords, Vector2_s32 dim, Vector2_s32 target) {
    for (s32 i = 0; i < dim.x; i++) {
        for (s32 j = 0; j < dim.y; j++) {
            if (coords.x + i == target.x && coords.y + j == target.y)
                return true;
        }
    }
    return false;
}

internal void
menu_update_active(Menu_Input input, Vector2_s32 section_dim, Vector2_s32 e[2]) {
    if (on_down(input.up)    && input.active_ptr->y - section_dim.y >= e[0].y) {
        input.active_ptr->y -= section_dim.y;
    }
    if (on_down(input.down)  && input.active_ptr->y + section_dim.y < e[1].y) {
        input.active_ptr->y += section_dim.y;
    }
    if (on_down(input.left)  && input.active_ptr->x - section_dim.x >= e[0].x) {
        input.active_ptr->x -= section_dim.x;
    }
    if (on_down(input.right) && input.active_ptr->x + section_dim.x < e[1].x) {
        input.active_ptr->x += section_dim.x;
    }
}

internal void
menu_text(Menu *menu, const char *text, Vector4 color, Vector2_s32 section_coords, Vector2_s32 section_dim) {
    Vector2 coords = {
        (menu->rect.dim.x / menu->sections.x) * section_coords.x + menu->rect.coords.x,
        (menu->rect.dim.y / menu->sections.y) * section_coords.y + menu->rect.coords.y
    };
    
    Vector2 dim = {
        (menu->rect.dim.x / menu->sections.x) * section_dim.x,
        (menu->rect.dim.y / menu->sections.y) * section_dim.y
    };

    float32 pixel_height = dim.y;
    if (dim.x < dim.y) pixel_height = dim.x;
    pixel_height *= 0.8f;

    Vector2 text_dim = get_string_dim(menu->font, text, pixel_height, color);
    Vector2 text_coords = {};
    text_coords.x = coords.x + (dim.x / 2.0f) - (text_dim.x / 2.0f);
    text_coords.y = coords.y + (dim.y / 2.0f) + (text_dim.y / 2.0f);

    draw_string(menu->font, text, text_coords, pixel_height, color);
}

internal bool8
menu_button(Menu *menu, const char *text, Menu_Input input, 
            Vector2_s32 section_coords, Vector2_s32 section_dim, Vector2_s32 draw_dim) {

    Vector2 coords = {
        (menu->rect.dim.x / menu->sections.x) * section_coords.x + menu->rect.coords.x,
        (menu->rect.dim.y / menu->sections.y) * section_coords.y + menu->rect.coords.y
    };
    
    Vector2 dim = {
        (menu->rect.dim.x / menu->sections.x) * draw_dim.x,
        (menu->rect.dim.y / menu->sections.y) * draw_dim.y
    };
    
    Draw_Button button = {
        menu->button_style.default_back_color,
        menu->button_style.default_text_color,

        menu->button_style.active_back_color,
        menu->button_style.active_text_color,

        false,

        coords,
        dim,

        menu->font,
        text
    };
    
    bool8 button_pressed = false;
    if (input.active_input_type == KEYBOARD_INPUT && menu_in_dim(section_coords, section_dim, input.active)) {
        menu_update_active(input, section_dim, menu->hot);
        button.active = true;
        if (input.select)
            button_pressed = true;
    }  else if (input.active_input_type == MOUSE_INPUT && coords_in_rect(input.mouse, coords, dim)) {
        button.active = true;
        if (input.select)
            button_pressed = true;
    }

    draw_button(button);
    
    return button_pressed;
}

internal bool8
menu_button(Menu *menu, const char *text, Menu_Input input, Vector2_s32 section_coords, Vector2_s32 section_dim) {
    return menu_button(menu, text, input, section_coords, section_dim, section_dim);
}

internal void
draw_textbox(const Draw_Textbox textbox) {
    float32 pixel_height = textbox.dim.y;
    if (textbox.dim.x < textbox.dim.y) 
        pixel_height = textbox.dim.x;
    pixel_height *= 0.8f;

    Vector2 text_dim = get_string_dim(textbox.font, textbox.text, pixel_height, textbox.text_color);
    Vector2 text_coords = {};
    text_coords.x = textbox.coords.x + (textbox.dim.x / 2.0f) - (text_dim.x / 2.0f);
    text_coords.y = textbox.coords.y + (textbox.dim.y / 2.0f) + (text_dim.y / 2.0f);

    draw_rect(textbox.coords, 0, textbox.dim, textbox.back_color);
    draw_string(textbox.font, textbox.text, text_coords, pixel_height, textbox.text_color);
}

internal void
menu_textbox(Menu *menu) {
    Draw_Textbox box = {
        menu->button_style.default_back_color,
        menu->button_style.default_text_color,
        
        menu->rect.coords,
        menu->button_style.dim,

        0.0f,

        menu->font,
        0
    };

    draw_textbox(box);
}

//
// Onscreen Notifications
//

internal void
add_onscreen_notification(Onscreen_Notifications *n, const char *not) {
    if (n->lines == ARRAY_COUNT(n->memory)) {
        print("add_onscreen_notification(): too many notfications\n");
        return;
    }

    n->times[n->lines] = 1.0f;
    n->colors[n->lines] = n->text_color;

    u32 not_length = get_length(not);
    for (u32 ch = 0; ch < not_length; ch++) {
        n->memory[n->lines][ch] = not[ch];
    }   
    n->lines++;
}

internal void
update_onscreen_notifications(Onscreen_Notifications *n, float32 frame_time_s) {
    for (u32 i = 0; i < n->lines; i++) {
        n->times[i] -= frame_time_s;

        n->colors[i].a = n->times[i];

        if (n->times[i] <= 0.0f) {
            n->colors[i].a = 0.0f;

            // Last line that was added is dissappearing so you can start adding from the beginning again.
            if (i == n->lines - 1) {
                n->lines = 0;
            }
        }
    }
}

internal float32
get_pixel_height(Vector2 box) {
    float32 pixel_height = box.y;
    if (box.x < box.y) pixel_height = box.x;
    pixel_height *= 0.8f;
    return pixel_height;
}

internal void
draw_onscreen_notifications(Onscreen_Notifications *n, Vector2_s32 window_dim, float32 frame_time_s) {
    if (n->font == 0) {
        logprint("draw_onscreen_notifications()", "Must set font in\n");
        return;
    }

    update_onscreen_notifications(n, frame_time_s);

    float32 pixel_height = get_pixel_height(cv2(window_dim) * 0.1f);

    float32 above_text_coord = 0.0f;
    for (u32 i = 0; i < n->lines; i++) {
        Vector2 text_dim = get_string_dim(n->font, n->memory[i], pixel_height, n->text_color);
        Vector2 text_coords = {};
        text_coords.x = (window_dim.x / 2.0f) - (text_dim.x / 2.0f);
        text_coords.y = above_text_coord + text_dim.y + 10.0f;

        if (n->times[i] == 0.0f)
            print("yo\n");

        draw_string(n->font, n->memory[i], text_coords, pixel_height, n->colors[i]);

        above_text_coord = text_coords.y;
    }
}
