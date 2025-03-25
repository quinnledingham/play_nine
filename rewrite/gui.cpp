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
        float32 pixel_height = rect.dim.y * 0.8f;

        String_Draw_Info info = get_string_draw_info(draw_ctx.font_id, label, -1, pixel_height);
        Vector2 text_coords = rect.coords + get_centered_text_coords(info, rect.dim, text_align);
        Vector4 text_color = style.text_colors[state];

        gfx_scissor_push(rect.coords, rect.dim);
        draw_text(label, text_coords, pixel_height, text_color); // text
        gfx_scissor_pop();
    }
}