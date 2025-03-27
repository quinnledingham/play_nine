// WARNING: centers vertically using baseline
inline Vector2
get_centered_text_coords(String_Draw_Info string_info, Vector2 dim, u32 text_align) {
    Vector2 coords = { 0, 0 };
    Vector2 text_coords = {};
    
    switch(text_align) {
        case ALIGN_CENTER:
            text_coords.x = coords.x + (dim.x / 2.0f) - (string_info.dim.x / 2.0f);
            text_coords.y = coords.y + (dim.y / 2.0f) - (string_info.dim.y / 2.0f);
            break;
        case ALIGN_RIGHT:
            text_coords.x = coords.x + (dim.x)        - (string_info.dim.x);
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
draw_button(const Draw_Style style, const u32 state, Rect rect, const char *label, const u32 text_align) {
    Vector4 back_color = style.background_colors[state];
    draw_rect(rect.coords, rect.dim, back_color); // back

    if (label) {
        float32 pixel_height;
        if (rect.dim.y < rect.dim.x)
            pixel_height = rect.dim.y * 0.8f;
        else
            pixel_height = rect.dim.x * 0.8f;

        String_Draw_Info info = get_string_draw_info(draw_ctx.font_id, label, -1, pixel_height);
        Vector2 text_coords = rect.coords + get_centered_text_coords(info, rect.dim, text_align);
        Vector4 text_color = style.text_colors[state];

        gfx_scissor_push(rect.coords, rect.dim);
        draw_text(label, text_coords, pixel_height, text_color); // text
        gfx_scissor_pop();
    }
}

internal void
draw_gui(GUI *gui) {
    draw_rect({0, 0}, cv2(gfx.window.dim), gui->background_color);

    Vector2 gui_dim_pixels = gui->dim * cv2(gfx.window.dim);
    float32 x_coord = float32((gfx.window.dim.x/2) - (gui_dim_pixels.x/2));
    float32 y_coord = float32((gfx.window.dim.y/2) - (gui_dim_pixels.y/2));
    gui->coords = {x_coord, y_coord};
    draw_rect(gui->coords, gui_dim_pixels, gui->back_color);

    gui->segment_dim = gui_dim_pixels / cv2(gui->segments);

    gui->index = 1; 
}

internal bool8
coords_in_rect(Vector2 in, Vector2 coords, Vector2 dim) {
    if (in.x >= coords.x && in.x <= coords.x + dim.x &&
        in.y >= coords.y && in.y <= coords.y + dim.y)
        return true;
    return false;
}

internal void
gui_do_mouse_input(GUI *gui, Vector2 coords, Vector2 dim) {
    if (coords_in_rect(app_input.mouse.coords, coords, dim)) {
        gui->hover = gui->index;
        // don't hover anyways
        //if (gui->active != 0 && !gui->enabled)

        // don't hover other components when one is pressed
        if (gui->pressed && gui->pressed != gui->index)
            gui->hover = 0;
    } else if (gui->hover == gui->index) {
        gui->hover = 0;
    }
}

// does mouse by default but for keyboard input to work
// gui->hover has to be set before. It does the logic of when keyboard is clicked
// but it does not know which one is being hovered.
internal u32
gui_update(GUI *gui, Vector2 coords, Vector2 dim) {
    u32 state = GUI_DEFAULT; // the state of the calling gui component        

    Button gui_select = {};
    switch(app_input.last_input_type) {
        //case CONTROLLER_INPUT:
        //case KEYBOARD_INPUT: gui_select = *gui->input.select;     break;
        case IN_MOUSE: gui_select = app_input.mouse.left; break;
    }

    if (app_input.last_input_type == IN_MOUSE)
        gui_do_mouse_input(gui, coords, dim);
    
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
gui_button(GUI *gui, const char *label, Vector2_s32 segment_coords) {
    Rect rect = {};
    rect.coords = gui->coords + (cv2(segment_coords) * gui->segment_dim);
    rect.dim = gui->segment_dim;
    u32 state = gui_update(gui, rect.coords, rect.dim);
    draw_button(gui->style, state, rect, label, ALIGN_CENTER); 

    gui->index++;

    bool8 button_pressed = false;
    if (state == GUI_ACTIVE) {
        button_pressed = true;
        gui->pressed = 0;
        gui->active = 0;
    }
    return button_pressed;
}