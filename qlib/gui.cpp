// WARNING: centers vertically using baseline
inline Vector2
get_centered_text_coords(Font *font, const char *text, float32 pixel_height, Vector2 dim, u32 text_align) {
    Vector2 coords = { 0, 0 };
    Vector2 text_coords = {};

    String_Draw_Info string_info = get_string_draw_info(font, text, -1, pixel_height);

    switch(text_align) {
        case ALIGN_CENTER:
        text_coords.x = coords.x + (dim.x / 2.0f) - (string_info.dim.x      / 2.0f) + string_info.baseline.x;
        text_coords.y = coords.y + (dim.y / 2.0f) - (string_info.baseline.y / 2.0f) + string_info.baseline.y;
        break;

        case ALIGN_RIGHT:
        text_coords.x = coords.x + (dim.x)        - (string_info.dim.x)             + string_info.baseline.x;
        text_coords.y = coords.y + (dim.y / 2.0f) - (string_info.baseline.y / 2.0f) + string_info.baseline.y;
        break;
        
        case ALIGN_LEFT:
        text_coords.x = coords.x + string_info.baseline.x;
        text_coords.y = coords.y + (dim.y / 2.0f) - (string_info.baseline.y / 2.0f) + string_info.baseline.y;
        break;
    }
    
    return text_coords;
}

internal void
draw_button(const Draw_Style style, const u32 state, Rect rect, const char *label, Font *font, const u32 text_align) {
    Vector4 back_color = style.background_colors[state];
    draw_rect(rect.coords, 0, rect.dim, back_color); // back
    
    if (label) {
        float32 pixel_height = rect.dim.y * 0.8f;

        Vector2 text_coords = rect.coords + get_centered_text_coords(font, label, pixel_height, rect.dim, text_align);
        Vector4 text_color = style.text_colors[state];

        render_set_scissor((s32)floor(rect.coords.x), (s32)floor(rect.coords.y), (u32)ceil(rect.dim.x), (u32)ceil(rect.dim.y));
        draw_string(font, label, text_coords, pixel_height, text_color); // text
        render_set_scissor(0, 0, window_dim.x, window_dim.y);
    }
}
/*
internal void
draw_checkbox(const Draw_Checkbox checkbox) {

}*/

internal float32
draw_textbox(Draw_Textbox textbox) {
    float32 padding = textbox.dim.y * 0.05f;
    Vector2 label_coords = textbox.coords + Vector2{ padding, padding };
    float32 label_height = textbox.dim.y * 0.5f;
    Vector2 label_dim = { textbox.dim.x, label_height };
    float32 pixel_height = textbox.dim.y * 0.8f;

    Vector4 back_color = textbox.style.background_colors[textbox.state];
    Vector4 text_color = textbox.style.text_colors[textbox.state];

    String_Draw_Info string_info = get_string_draw_info(textbox.font, textbox.text, -1, pixel_height);
    Vector2 text_coords = textbox.coords + get_centered_text_coords(textbox.font, textbox.text, pixel_height, textbox.dim, textbox.text_align);

    String_Draw_Info string_info_cursor = get_string_draw_info(textbox.font, textbox.text, textbox.cursor_position, pixel_height);
    Vector2 cursor_coords = text_coords;
    cursor_coords.x += string_info_cursor.dim.x;
    cursor_coords.y = textbox.coords.y;

    float32 shift = textbox.text_shift;
    if (textbox.state == GUI_ACTIVE) {
        if (cursor_coords.x - shift + textbox.cursor_width >= textbox.coords.x + textbox.dim.x) {
            shift = (cursor_coords.x + textbox.cursor_width) - (textbox.coords.x + textbox.dim.x);
        } else if (cursor_coords.x - shift <= textbox.coords.x) {
            shift = cursor_coords.x - textbox.coords.x;
        }

        text_coords.x -= shift;
        cursor_coords.x -= shift;
    }

    if (string_info.dim.x < textbox.dim.x)
        shift = 0.0f;

    // Back
    draw_rect(textbox.coords, 0, textbox.dim, back_color);

    // Label
    if (textbox.label != 0) {
        Vector4 label_text_color = text_color;
        label_text_color.a = 0.8f;
        String_Draw_Info label_info = get_string_draw_info(textbox.font, textbox.label, -1, label_height);
        label_coords.y += label_info.baseline.y;
        draw_string(textbox.font, textbox.label, label_coords, label_height, label_text_color);
    }

    // Text
    render_set_scissor((s32)floor(textbox.coords.x), (s32)floor(textbox.coords.y), (u32)ceil(textbox.dim.x), (u32)ceil(textbox.dim.y) );
    draw_string(textbox.font, textbox.text, text_coords, pixel_height, text_color);
    render_set_scissor(0, 0, window_dim.x, window_dim.y);

    // Cursor
    if (textbox.state == GUI_ACTIVE) // clicked on
        draw_rect(cursor_coords, 0.0f, { textbox.cursor_width, textbox.dim.y }, textbox.cursor_color); // cursor

    return shift;
}

//
// GUI
//

/*
void
ui_button(UI *ui, Vector2 coords, Vector2 dim, const char *text) {

}
*/

internal u32
gui_update(GUI *gui, Vector2 coords, Vector2 dim) {
    u32 state = GUI_DEFAULT; // the state of the calling gui component

    Button gui_select = {};
    if (*gui->input.active_input_type == KEYBOARD_INPUT) {
        gui_select = *gui->input.select;

        if (gui->hover == 0)
            gui->hover = 1;
    } else if (*gui->input.active_input_type == MOUSE_INPUT) {
        gui_select = *gui->input.mouse_left;

        if (coords_in_rect(*gui->input.mouse, coords, dim)) {
            gui->hover = gui->index;
        } else if (gui->hover == gui->index) {
            gui->hover = 0;
        }
    }

    if (gui->hover == gui->index) {
        state = GUI_HOVER;

        if (on_down(gui_select)) {
            gui->pressed = gui->hover;
        }
    }
        
    if (gui->pressed == gui->index) {
        state = GUI_PRESSED;

        if (on_up(gui_select)) {
            if (gui->hover == gui->index) {
                gui->active = gui->pressed;
                state = GUI_ACTIVE;
            } else {
                gui->pressed = 0;
                gui->active = 0;
            }
        }
    }
    
    if (gui->active == gui->index) {
        state = GUI_ACTIVE;
    } 
    
    return state;
}

internal bool8
gui_button(GUI *gui, Draw_Style style, const char *text, Vector2 coords, Vector2 dim) {
    u32 state = gui_update(gui, coords, dim);
    Rect rect = {};
    rect.coords = coords;
    rect.dim = dim;
    draw_button(style, state, rect, text, gui->font, gui->text_align);

    bool8 button_pressed = false;
    if (gui->index == gui->active) {
        button_pressed = true;
        gui->pressed = 0;
        gui->active = 0;
    }

    gui->index++;

    return button_pressed;
} 

internal bool8
gui_checkbox(GUI *gui, Draw_Style style, const char *label, Vector2 coords, Vector2 dim) {
    bool8 checkbox_pressed = false;
    return checkbox_pressed;
}

// move cursor to mouse based on how the textbox will be drawn
internal u32
get_textbox_cursor_position(const Draw_Textbox *box, Vector2_s32 mouse_coords) {
    u32 max_length = get_length(box->text);
    u32 cursor_pos = 0;
    while(1) {
        //Vector2 cursor_dim = get_string_dim(box->font, box->text, cursor_pos, box->dim.y);
        String_Draw_Info string_info = get_string_draw_info(box->font, box->text, cursor_pos, box->dim.y);
        if (mouse_coords.x <= (s32)string_info.dim.x + box->coords.x || cursor_pos >= max_length)
            break;
        else
            cursor_pos++;
    }
    return cursor_pos;
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
gui_textbox(GUI *gui, Draw_Style style, const char *label, const char *dest, Vector2 coords, Vector2 dim) {
    Draw_Textbox box = {
        style,
        GUI_DEFAULT,

        { 255, 255, 255, 1 },
        gui->edit.cursor_position,
        dim.y * 0.05f,
        gui->edit.shift,

        coords,
        dim,

        gui->font,
        dest,
        gui->text_align,

        label
    };

    box.state = gui_update(gui, coords, dim);

    local_persist u32 last_state = GUI_DEFAULT;

    bool8 new_text = false;

    if (gui->index == gui->active) {
        if ((on_up(*gui->input.select) || on_up(*gui->input.mouse_left)) && gui->edit.index != gui->active) {
            box.text_shift = 0.0f;
            gui->edit.cursor_position = get_length(dest);
            platform_memory_set(gui->edit.text, 0, TEXTBOX_SIZE);
            platform_memory_copy(gui->edit.text, (void*)dest, gui->edit.cursor_position);
            gui->edit.index = gui->active;
        }

        app_input_buffer = true;
        box.text = gui->edit.text;

        //if (input.pressed && input.active_input_type == MOUSE_INPUT)
        //    gui->edit.cursor_position = get_textbox_cursor_position(&box, input.mouse);

        s32 ch = 0;
        s32 i = 0;
        while(i < *gui->input.buffer_index) {
            ch = gui->input.buffer[i++];
            switch(ch) {
                case 27: // Esc: Close textbox
                    box.state = GUI_DEFAULT;
                    gui->pressed = 0;
                    gui->active = 0;
                    gui->edit.index = 0;
                break;
            
                default: {
                    if (update_textbox(gui->edit.text, TEXTBOX_SIZE - 1, &gui->edit.cursor_position, ch)) {
                        platform_memory_set((void*)dest, 0, TEXTBOX_SIZE);
                        platform_memory_copy((void*)dest, gui->edit.text, get_length(gui->edit.text));
                        box.state = GUI_DEFAULT;
                        gui->pressed = 0;
                        gui->active = 0;
                        gui->edit.index = 0;
                        new_text = true;
                    }
                } break;
            }
        } 
    }
    
    gui->edit.shift = draw_textbox(box);

    last_state = box.state;

    gui->index++;

    return new_text;
}

//
// Menu
//

inline Vector2
get_screen_coords(Menu *menu, Vector2_s32 section_coords) {
    Vector2 coords = {
        (menu->gui.rect.dim.x / menu->sections.x) * section_coords.x + menu->gui.rect.coords.x,
        (menu->gui.rect.dim.y / menu->sections.y) * section_coords.y + menu->gui.rect.coords.y
    };
    return coords;
}

inline Vector2
get_screen_dim(Menu *menu, Vector2_s32 draw_dim) {
    Vector2 dim = {
        (menu->gui.rect.dim.x / menu->sections.x) * draw_dim.x,
        (menu->gui.rect.dim.y / menu->sections.y) * draw_dim.y
    };
    return dim;
}

inline void
menu_rect(Menu *menu, Vector2_s32 section_coords, Vector2_s32 section_dim, Vector4 color) {
    draw_rect(get_screen_coords(menu, section_coords), 0, get_screen_dim(menu, section_dim), color );
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

// e is the coordniates of the menu that can be active (interact_region)
internal void
menu_update_hot(GUI_Input *input, Vector2_s32 *hover_updated, Vector2_s32 section_coords, Vector2_s32 section_dim, Vector2_s32 e[2]) {
    Vector2_s32 sub_value = *hover_updated - section_coords;
    Vector2_s32 add_value = section_dim - sub_value;
    sub_value.x += 1;
    sub_value.y += 1;

    if (on_down(*input->up)    && hover_updated->y - sub_value.y >= e[0].y) {
        hover_updated->y -= sub_value.y;
    }
    if (on_down(*input->down)  && hover_updated->y + add_value.y < e[1].y) {
        hover_updated->y += add_value.y;
    }
    if (on_down(*input->left)  && hover_updated->x - sub_value.x >= e[0].x) {
        hover_updated->x -= sub_value.x;
    }
    if (on_down(*input->right) && hover_updated->x + add_value.x < e[1].x) {
        hover_updated->x += add_value.x;
    }
}

internal void
do_menu_update(Menu *menu, Vector2_s32 section_coords, Vector2_s32 section_dim) {
    switch(*menu->gui.input.active_input_type) {
        case KEYBOARD_INPUT: {
            if (menu_in_dim(section_coords, section_dim, menu->hover_section)) {
                menu_update_hot(&menu->gui.input, &menu->hover_section_updated, section_coords, section_dim, menu->interact_region);
                menu->gui.hover = menu->gui.index;
            }
        } break;

        case MOUSE_INPUT: {
            menu->gui.hover = menu->gui.index;
            menu->hover_section_updated = section_coords;
        } break;
    }
}

internal void
menu_text(Menu *menu, const char *text, Vector4 color, Vector2_s32 section_coords, Vector2_s32 section_dim) {
    Vector2 coords = get_screen_coords(menu, section_coords);
    Vector2 dim = get_screen_dim(menu, section_dim);
    
    float32 pixel_height = dim.y;
    if (dim.x < dim.y) 
        pixel_height = dim.x;
    pixel_height *= 0.8f;

    Vector2 text_coords = coords + get_centered_text_coords(menu->gui.font, text, pixel_height, dim, menu->gui.text_align);
    render_set_scissor((s32)floor(coords.x), (s32)floor(coords.y), (u32)ceil(dim.x), (u32)ceil(dim.y) );
    draw_string(menu->gui.font, text, text_coords, pixel_height, color);
    render_set_scissor(0, 0, window_dim.x, window_dim.y);
}

internal bool8
menu_button(Menu *menu, const char *text, Vector2_s32 section_coords, Vector2_s32 section_dim, Vector2_s32 draw_dim) {
    Vector2 coords = get_screen_coords(menu, section_coords);
    Vector2 dim = get_screen_dim(menu, draw_dim);

    do_menu_update(menu, section_coords, section_dim);
    
    return gui_button(&menu->gui, menu->gui.style, text, coords, dim);
 }

internal bool8
menu_button(Menu *menu, const char *text, Vector2_s32 section_coords, Vector2_s32 section_dim) {
    return menu_button(menu, text, section_coords, section_dim, section_dim);
}

internal bool8
menu_textbox(Menu *menu, const char *label, const char *dest, Vector2_s32 section_coords, Vector2_s32 section_dim, Vector2_s32 draw_dim) {
    Vector2 coords = get_screen_coords(menu, section_coords);
    Vector2 dim = get_screen_dim(menu, draw_dim);

    do_menu_update(menu, section_coords, section_dim);
    
    return gui_textbox(&menu->gui, menu->gui.style, label, dest, coords, dim);
}

// no label and no draw dim
internal bool8
menu_textbox(Menu *menu, const char *dest, Vector2_s32 section_coords, Vector2_s32 section_dim) {
    return menu_textbox(menu, 0, dest, section_coords, section_dim, section_dim);
}

// no draw dim 
internal bool8
menu_textbox(Menu *menu, const char * label, const char *dest, Vector2_s32 section_coords, Vector2_s32 section_dim) {
    return menu_textbox(menu, label, dest, section_coords, section_dim, section_dim);
}

// no label
internal bool8
menu_textbox(Menu *menu, const char *dest, Vector2_s32 section_coords, Vector2_s32 section_dim, Vector2_s32 draw_dim) {
    return menu_textbox(menu, 0, dest, section_coords, section_dim, draw_dim);
}

//
// Onscreen Notifications
//

internal void
add_onscreen_notification(Onscreen_Notifications *n, const char *noti) {
    if (n->lines == ARRAY_COUNT(n->memory)) {
        print("add_onscreen_notification(): too many notfications\n");
        return;
    }

    n->times[n->lines] = 2.0f;
    n->colors[n->lines] = n->text_color;
    platform_memory_set(n->memory[n->lines], 0, sizeof(n->memory[n->lines]));

    u32 not_length = get_length(noti);
    for (u32 ch = 0; ch < not_length; ch++) {
        n->memory[n->lines][ch] = noti[ch];
    }   
    n->lines++;
}

internal void
update_onscreen_notifications(Onscreen_Notifications *n, float32 frame_time_s) {
    for (u32 i = 0; i < n->lines; i++) {
        if (frame_time_s < 1.0f)
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
        //Vector2 text_dim = get_string_dim(n->font, n->memory[i], pixel_height, n->text_color);
        String_Draw_Info string_info = get_string_draw_info(n->font, n->memory[i], -1, pixel_height);
        Vector2 text_coords = {};
        text_coords.x = (window_dim.x / 2.0f) - (string_info.dim.x / 2.0f) + string_info.baseline.x;
        text_coords.y = above_text_coord + string_info.baseline.y + 10.0f;

        if (n->times[i] == 0.0f)
            print("yo\n");

        draw_string(n->font, n->memory[i], text_coords, pixel_height, n->colors[i]);

        above_text_coord = text_coords.y;
    }
}
