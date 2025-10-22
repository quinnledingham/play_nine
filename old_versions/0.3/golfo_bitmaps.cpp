//
// creating card bitmpas
//

internal Texture
create_string_into_bitmap(Font *font, float32 pixel_height, const char *str) {    
    float32 scale = get_scale_for_pixel_height(font, pixel_height);
    String_Draw_Info draw_info = get_string_draw_info(font, str, -1, pixel_height);
    Texture bitmap = blank_bitmap((s32)ceilf(draw_info.dim.x), (s32)ceilf(draw_info.dim.y), 1);

    float32 current_point = draw_info.baseline.x;

    u32 i = 0;
    while (str[i] != 0 ) {
        Font_Char_Texture *fbitmap = load_font_char_bitmap(font, str[i], scale);
        Font_Char *font_char = fbitmap->font_char;

        Vector2 char_coords = { 
            current_point + (font_char->lsb * scale), 
            draw_info.baseline.y + (float32)fbitmap->bb_0.y
        };
        copy_blend_bitmap(bitmap, fbitmap->bitmap, cv2(char_coords));

        s32 kern = get_codepoint_kern_advance(font->info, str[i], str[i + 1]);
        current_point += scale * (kern + font_char->ax);
        
        i++;
    }

    return bitmap;
}

internal Texture
create_circle_bitmap(Vector2_s32 dim) {
    Texture bitmap = blank_bitmap(dim.x, dim.y, 1);
    u8 *dest_ptr = bitmap.memory;

    s32 x1 = dim.x / 2;
    s32 y1 = dim.y / 2;
    s32 r = dim.x / 2;

    for (s32 y2 = 0; y2 < dim.y; y2++) {
        for (s32 x2 = 0; x2 < dim.x; x2++) {
            s32 dx = x2 - x1;
            s32 dy = y2 - y1;
            if ((dx * dx + dy * dy) <= (r * r)) {
                *dest_ptr = 0xFF;
            }

            dest_ptr += bitmap.channels;
        }

        dest_ptr = bitmap.memory + (y2 * bitmap.pitch);
    }

    return bitmap;
}

internal void
add_balls_line(Texture bitmap, Texture circle_bitmap, s32 x, s32 number, Vector3 color) {
    float32 radius = (circle_bitmap.width  / 2.0f);
    float32 dots_height_percent = 0.9f;
    float32 dots_height = (bitmap.height * dots_height_percent);
    float32 up = bitmap.height * (1 - dots_height_percent) / 2.0f;
    for (s32 i = 1; i <= number; i++) {
        float32 y = (dots_height * (float32(i) / float32(number + 1))) - radius + up;
        bitmap_copy_text(bitmap, circle_bitmap, { x, s32(y) }, get_color(color));
    }
}

internal void
add_balls_to_bitmap(Texture bitmap, Texture circle_bitmap, s32 number) {
    float32 x = 0.0f;
    float32 y = 0.0f;

    s32 *design = ball_rows[number];
    s32 columns = 0; // count how many columns are in this design to only space for that many
    for (u32 i = 0; i < 3; i++) {
        if (design[i] > 0)
            columns++;
    }

    // draw each column that is not zero
    s32 columns_index = 1; 
    float32 radius = (circle_bitmap.width  / 2.0f);
    float32 dots_width_percent = 0.9f;
    float32 dots_width = (bitmap.width * dots_width_percent);
    float32 left = bitmap.width * (1 - dots_width_percent) / 2.0f;
    for (u32 i = 0; i < 3; i++) {
        if (design[i] > 0) {
            x = (dots_width * (columns_index++ / float32(columns + 1))) - radius + left;
            add_balls_line(bitmap, circle_bitmap, s32(x), design[i], ball_colors[number]);
        }
    }
}

internal Texture
create_card_bitmap(Font *font, s32 number, Texture circle_bitmap, Vector2_s32 card_dim) {
    s32 number_padding = 20;
    float32 number_pixel_height = 60.0f;

    Texture bitmap = blank_bitmap(card_dim.x, card_dim.y, 4);    
    Texture front = bitmap_resized(find_bitmap(BITMAP_FRONT), bitmap.dim);
    memcpy(bitmap.memory, front.memory, front.width * front.height * front.channels);

    if (number < 11 || number == 13)
        add_balls_to_bitmap(bitmap, circle_bitmap, number);

    // adding front design
    u32 front_bitmap_id = 0;
    switch(number) {
        case  0: front_bitmap_id = BITMAP_FRONT_0;  break;
        case 11: front_bitmap_id = BITMAP_FRONT_11; break;
        case 12: front_bitmap_id = BITMAP_FRONT_12; break;
    }

    if (front_bitmap_id != 0) {
        Texture front_bitmap = bitmap_resized(find_bitmap(front_bitmap_id), bitmap.dim);
        Vector2_s32 center = centered(bitmap.dim, front_bitmap.dim);
        copy_blend_bitmap(bitmap, front_bitmap, center);
    }

    // add string numbers to card
    if (number == 13)
        number = -5;

    char str[3] = {};
    switch(number) {
        case -5: str[0] = '-'; str[1] = '5'; break;
        case 10: str[0] = '1'; str[1] = '0'; break;
        case 11: str[0] = '1'; str[1] = '1'; break;
        case 12: str[0] = '1'; str[1] = '2'; break;
        default: str[0] = number + 48;       break;
    }

    Texture str_bitmap = create_string_into_bitmap(font, number_pixel_height, str);

    Vector3 text_color = ball_colors[number];
    if (number == -5)
        text_color = { 255, 0, 0 };

    Vector2_s32 bottom_right = {
        bitmap.width  - str_bitmap.width  - number_padding,
        bitmap.height - str_bitmap.height - number_padding
    };

    bitmap_convert_channels(&str_bitmap, 4);
    
    copy_blend_bitmap(bitmap, str_bitmap, { number_padding, number_padding });
    copy_blend_bitmap(bitmap, str_bitmap, bottom_right);

    free(str_bitmap.memory);  
    return bitmap;
}

internal Texture_Atlas
init_card_atlas(Font *font) {
    // 1.4317439794540405, 2
    Vector2_s32 card_dim = { 358, 500 };
    s32 ball_radius = 50;

    Texture_Atlas atlas = create_texture_atlas(card_dim.x * 8, card_dim.y * 3, 4);
    Texture circle_bitmap = create_circle_bitmap({ ball_radius, ball_radius });

    for (s32 i = 0; i <= 13; i++) {
        Texture bitmap = create_card_bitmap(font, i, circle_bitmap, card_dim);
        texture_atlas_add(&atlas, &bitmap);
        //free(bitmap.memory);
        stbi_image_free(bitmap.memory);
    }

    free(circle_bitmap.memory);
    return atlas;
}

internal void
draw_card_bitmaps() {
    Texture_Atlas *atlas = find_atlas(ATLAS_CARDS);

    Vector2 pos = { 0, 0 };
    float32 percent = (float)gfx.window.dim.x / (float)atlas->bitmap.width;
    Vector2 dim = { (float32)gfx.window.dim.x, percent * atlas->bitmap.height };

    Local local = {};
    local.text.x = 2;
    gfx_ubo(GFXID_LOCAL, &local, 0);

    gfx_bind_atlas(atlas);

    Object object = {};
    object.model = create_transform_m4x4(pos, dim);
    object.index = 0;
    vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
    vulkan_draw_mesh(&draw_ctx.square);
}
