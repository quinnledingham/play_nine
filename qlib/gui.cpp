internal void
draw_button(const Draw_Button button) {
    Vector4 back_color = button.default_back_color;
    Vector4 text_color = button.default_text_color;
    
    if (button.hot) {
        back_color = button.active_back_color;
        text_color = button.active_text_color;
    }

    float32 pixel_height = button.dim.y;
    if (button.dim.x < button.dim.y) 
        pixel_height = button.dim.x;
    pixel_height *= 0.8f;

    Vector2 text_dim = get_string_dim(button.font, button.text, pixel_height, text_color);
    Vector2 text_coords = {};
    text_coords.x = button.coords.x + (button.dim.x / 2.0f) - (text_dim.x / 2.0f);
    text_coords.y = button.coords.y + (button.dim.y / 2.0f) + (text_dim.y / 2.0f);

    draw_rect(button.coords, 0, button.dim, back_color);                                           // back
    if (button.text) draw_string(button.font, button.text, text_coords, pixel_height, text_color); // text
}

internal void
draw_textbox(const Draw_Textbox textbox) {
    float32 pixel_height = textbox.dim.y;
    if (textbox.dim.x < textbox.dim.y) 
        pixel_height = textbox.dim.x;
    pixel_height *= 0.8f;

    Vector4 back_color = textbox.style.default_back;
    Vector4 text_color = textbox.style.default_text;

    if (textbox.hot) {
        back_color = textbox.style.hot_back;
        text_color = textbox.style.hot_text;
    } 

    if (textbox.active) {
        back_color = textbox.style.active_back;
        text_color = textbox.style.active_text;
    }

    Vector2 text_dim = get_string_dim(textbox.font, textbox.text, pixel_height, text_color);
    Vector2 text_coords = {};
    text_coords.x = textbox.coords.x + (textbox.dim.x / 2.0f) - (text_dim.x / 2.0f);
    text_coords.y = textbox.coords.y + (textbox.dim.y / 2.0f) + (text_dim.y / 2.0f);

    Vector2 text_dim_cursor = get_string_dim(textbox.font, textbox.text, textbox.cursor_position, pixel_height, text_color);
    Vector2 cursor_coords = text_coords;
    cursor_coords.x += text_dim_cursor.x;
    cursor_coords.y = textbox.coords.y;

    draw_rect(textbox.coords, 0, textbox.dim, back_color);
    if (textbox.active) // clicked on
        draw_rect(cursor_coords, 0.0f, { textbox.cursor_width, textbox.dim.y }, textbox.cursor_color); // cursor
    draw_string(textbox.font, textbox.text, text_coords, pixel_height, text_color);
}

internal bool8
menu_in_dim(Vector2_s32 coords, Vector2_s32 dim, Vector2_s32 target) {
    for (s32 i = 0; i < dim.x; i++) {
        for (s32 j = 0; j < dim.y; j++) {
            if (coords.x + i == target.x && coords.y + j == target.y) {
                return true;

            }
        }
    }
    return false;
}

// works if target is in dim
internal Vector2_s32
menu_get_dim(Vector2_s32 coords, Vector2_s32 dim, Vector2_s32 target) {
    Vector2_s32 result = {};

    Vector2_s32 abs_coords = target - coords;
    result = dim - abs_coords;
    
    return result;    
}

// e is the coordniates of the menu that can be active
internal void
menu_update_hot(Menu_Input input, Vector2_s32 coords, Vector2_s32 section_dim, Vector2_s32 e[2]) {
    Vector2_s32 sub_value = *input.hot_ptr - coords;
    Vector2_s32 add_value = section_dim - sub_value;
    sub_value.x += 1;
    sub_value.y += 1;

    if (on_down(input.up)    && input.hot_ptr->y - sub_value.y >= e[0].y) {
        input.hot_ptr->y -= sub_value.y;
    }
    if (on_down(input.down)  && input.hot_ptr->y + add_value.y < e[1].y) {
        input.hot_ptr->y += add_value.y;
    }
    if (on_down(input.left)  && input.hot_ptr->x - sub_value.x >= e[0].x) {
        input.hot_ptr->x -= sub_value.x;
    }
    if (on_down(input.right) && input.hot_ptr->x + add_value.x < e[1].x) {
        input.hot_ptr->x += add_value.x;
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
    if (dim.x < dim.y) 
        pixel_height = dim.x;
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
        menu->style.default_back,
        menu->style.default_text,

        menu->style.hot_back,
        menu->style.hot_text,

        false,

        coords,
        dim,

        menu->font,
        text
    };
    
    
    if (input.active_input_type == KEYBOARD_INPUT && menu_in_dim(section_coords, section_dim, input.hot)) {
        menu_update_hot(input, section_coords, section_dim, menu->interact_region);
        button.hot = true;
        if (input.select)
            menu->active_section = menu->hot_section;
    }  else if (input.active_input_type == MOUSE_INPUT && coords_in_rect(input.mouse, coords, dim)) {
        button.hot = true;
        if (input.select)
            menu->active_section = section_coords;
    }

    bool8 button_pressed = false;
    if (menu_in_dim(section_coords, section_dim, input.active)) {
        //button.active = true;
        button_pressed = true;
        menu->active_section = { -1, -1 };
    } 

    draw_button(button);
    
    return button_pressed;
}

internal bool8
menu_button(Menu *menu, const char *text, Menu_Input input, Vector2_s32 section_coords, Vector2_s32 section_dim) {
    return menu_button(menu, text, input, section_coords, section_dim, section_dim);
}

// default textbox updates
// input is a ascii value
internal bool32
update_textbox(char *buffer, u32 max_length, u32 *cursor_position, s32 input) {
    u32 current_length = get_length(buffer);

    switch(input) {
        
        case 8: { // Backspace: delete char
            if (*cursor_position == 0) return false;

            for(u32 i = *cursor_position; i < current_length; i++) {
                buffer[i - 1] = buffer[i];
            }
            buffer[current_length - 1] = 0;
            (*cursor_position)--;
        } break;

        case 13: { // Enter/Return: return true to tell calling code to do something
            return true;
        } break;

        case 37: { // Left
            if (*cursor_position != 0)
                (*cursor_position)--; // Left
        } break;

        case 39: { // Right
            if (*cursor_position != current_length)
                (*cursor_position)++; // Right
        } break;
        
        default: { // Add char to char_array in textbox
            if (current_length >= max_length) 
                return false;

            for(s32 i = current_length - 1; i >= (s32)(*cursor_position); i--) {
                buffer[i + 1] = buffer[i];
            }
            buffer[*cursor_position] = input;
            buffer[current_length + 1] = 0;
            (*cursor_position)++;
        } break;
    }

    return false;
}

internal bool8
menu_textbox(Menu *menu, const char *dest, Menu_Input input, Vector2_s32 section_coords, Vector2_s32 section_dim, Vector2_s32 draw_dim) {
    Vector2 coords = {
        (menu->rect.dim.x / menu->sections.x) * section_coords.x + menu->rect.coords.x,
        (menu->rect.dim.y / menu->sections.y) * section_coords.y + menu->rect.coords.y
    };
    
    Vector2 dim = {
        (menu->rect.dim.x / menu->sections.x) * draw_dim.x,
        (menu->rect.dim.y / menu->sections.y) * draw_dim.y
    };

    Draw_Textbox box = {
        menu->style,

        { 255, 255, 255, 1 },
        menu->edit.cursor_position,
        5.0f,
        0.0f,

        coords,
        dim,

        false,
        false,

        menu->font,
        dest
    };

    if (input.active_input_type == KEYBOARD_INPUT && menu_in_dim(section_coords, section_dim, input.hot)) {
        menu_update_hot(input, section_coords, section_dim, menu->interact_region);
        box.hot = true;
        if (input.select) {
            menu->active_section = menu->hot_section;
            menu->edit.cursor_position = get_length(dest);
            platform_memory_set(menu->edit.text, 0, TEXTBOX_SIZE);
            platform_memory_copy(menu->edit.text, (void*)dest, menu->edit.cursor_position);
        }
    }  else if (input.active_input_type == MOUSE_INPUT && coords_in_rect(input.mouse, coords, dim)) {
        box.hot = true;
        if (input.select) {
            menu->active_section = section_coords;
        }
    }

    bool8 new_text = false;

    if (menu_in_dim(section_coords, section_dim, input.active)) {
        app_input_buffer = true;
        box.active = true;
        box.text = menu->edit.text;

        s32 ch = 0;
        s32 i = 0;
        while(i < input.buffer_index) {
            ch = input.buffer[i++];
            switch(ch) {
                case 27: // Esc: Close textbox
                    box.active = false;
                    menu->active_section = { -1, -1 };
                break;
            
                default: {
                    if (update_textbox(menu->edit.text, TEXTBOX_SIZE - 1, &menu->edit.cursor_position, ch)) {
                        platform_memory_set((void*)dest, 0, TEXTBOX_SIZE);
                        platform_memory_copy((void*)dest, menu->edit.text, menu->edit.cursor_position);
                        box.active = false;
                        menu->active_section = { -1, -1 };
                        new_text = true;
                    }
                } break;
            }
        }
    } 

    draw_textbox(box);

    return new_text;
}

internal bool8
menu_textbox(Menu *menu, const char *text, Menu_Input input, Vector2_s32 section_coords, Vector2_s32 section_dim) {
    return menu_textbox(menu, text, input, section_coords, section_dim, section_dim);
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

    n->times[n->lines] = 2.0f;
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
    if (box.x < box.y) 
        pixel_height = box.x;
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
