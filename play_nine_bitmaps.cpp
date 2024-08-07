//
// creating card bitmpas
//

internal Vector4
get_color(u8 *ptr, u32 channels, Vector3 color) {
    switch(channels) {
        case 1: return { color.r, color.g, color.b, float32(*ptr) }; break;
        case 3: return { float32(ptr[0]), float32(ptr[1]), float32(ptr[2]), 0xFF }; break;
        case 4: return { float32(ptr[0]), float32(ptr[1]), float32(ptr[2]), float32(ptr[3]) }; break;
    }

    return { 0, 0, 0, 0 };
}

internal u8
blend_alpha(u8 a1, u8 a2) {
    return 255 - ((255 - a1) * (255 - a2) / 255);
}

internal Vector4
get_color(Vector4 c1, Vector4 c2) {
    float32 alpha = 255 - ((255 - c1.E[3]) * (255 - c2.E[3]) / 255);
    float32 red   = (c1.E[0] * (255 - c2.E[3]) + c2.E[0] * c2.E[3]) / 255;
    float32 green = (c1.E[1] * (255 - c2.E[3]) + c2.E[1] * c2.E[3]) / 255;
    float32 blue  = (c1.E[2] * (255 - c2.E[3]) + c2.E[2] * c2.E[3]) / 255;
    return {red, green, blue, alpha};
}

// color contains what to fill conversion from 1 channel to 4 channels with
internal void
copy_blend_bitmap(Bitmap dest, const Bitmap src, Vector2_s32 position, Vector3 color) {
    if (src.width == 0)
        return;

    u8 *dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch);
    u8 *src_ptr = src.memory;
    for (s32 y = 0; y <= src.height; y++) {
        for (s32 x = 0; x < src.width; x++) { 
            if (dest.channels == 1) {
                *dest_ptr = blend_alpha(*dest_ptr, *src_ptr);
            } else if (dest.channels == 4) { 
                Vector4 src_color = get_color(src_ptr, src.channels, color);
                Vector4 dest_color = get_color(dest_ptr, dest.channels, color);
                Vector4 blend_color = get_color(dest_color, src_color);

                dest_ptr[0] = (u8)blend_color.r;
                dest_ptr[1] = (u8)blend_color.g;
                dest_ptr[2] = (u8)blend_color.b;
                dest_ptr[3] = (u8)blend_color.a;
            }
            dest_ptr += dest.channels;
            src_ptr += src.channels;
        }   

        dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch) + (y * dest.pitch);
        src_ptr = src.memory + (y * src.pitch);

        if (dest_ptr > dest.memory + (dest.width * dest.channels) + (dest.height * dest.pitch) + (y * dest.pitch))
            ASSERT(0);
    }
}

internal Bitmap
create_string_into_bitmap(Font *font, float32 pixel_height, const char *str) {
    Bitmap bitmap = {};
    
    float32 scale = get_scale_for_pixel_height(font->info, pixel_height);
    String_Draw_Info draw_info = get_string_draw_info(font, str, -1, pixel_height);

    bitmap.width = (s32)ceilf(draw_info.dim.x);
    bitmap.height = (s32)ceilf(draw_info.dim.y);
    bitmap.channels = 1;
    bitmap.pitch = bitmap.width * bitmap.channels;
    bitmap.memory = (u8*)platform_malloc(bitmap.width * bitmap.height * bitmap.channels);
    memset(bitmap.memory, 0x00, bitmap.width * bitmap.height * bitmap.channels);

    float32 current_point = 0.0f;
    u32 i = 0;
    while (str[i] != 0 ) {
        Font_Char_Bitmap *fbitmap = load_font_char_bitmap(font, str[i], scale);
        Font_Char *font_char = fbitmap->font_char;

        Vector2 char_coords = { 
            draw_info.baseline.x + current_point + (font_char->lsb * scale), 
            draw_info.baseline.y + (float32)fbitmap->bb_0.y
        };
        copy_blend_bitmap(bitmap, fbitmap->bitmap, cv2(char_coords), { 0, 0, 0 });

        s32 kern = get_codepoint_kern_advance(font->info, str[i], str[i + 1]);
        current_point += scale * (kern + font_char->ax);
        
        i++;
    }

    return bitmap;
}

internal Bitmap
create_circle_bitmap(Vector2_s32 dim) {
    Bitmap bitmap   = {};
    bitmap.width    = dim.x;
    bitmap.height   = dim.y;
    bitmap.channels = 1;
    bitmap.pitch    = bitmap.width * bitmap.channels;
    bitmap.memory   = (u8*)platform_malloc(bitmap.width * bitmap.height * bitmap.channels);
    memset(bitmap.memory, 0x00, bitmap.width * bitmap.height * bitmap.channels);

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
add_balls_line(Bitmap bitmap, Bitmap circle_bitmap, s32 x, s32 number, Vector3 color) {
    float32 radius = (circle_bitmap.width  / 2.0f);
    float32 dots_height_percent = 0.9f;
    float32 dots_height = (bitmap.height * dots_height_percent);
    float32 up = bitmap.height * (1 - dots_height_percent) / 2.0f;
    for (s32 i = 1; i <= number; i++) {
        float32 y = (dots_height * (float32(i) / float32(number + 1))) - radius + up;
        copy_blend_bitmap(bitmap, circle_bitmap, { x, s32(y) }, color);
    }
}

internal void
add_balls_to_bitmap(Bitmap bitmap, Bitmap circle_bitmap, s32 number) {
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

internal Bitmap
create_card_bitmap(Font *font, s32 number, Bitmap circle_bitmap) {
    Bitmap bitmap   = {};
    bitmap.width    = 1000;
    bitmap.height   = 1600;
    bitmap.channels = 4;
    bitmap.pitch    = bitmap.width * bitmap.channels;
    bitmap.memory   = (u8*)platform_malloc(bitmap.width * bitmap.height * bitmap.channels);
    memset(bitmap.memory, 0xFF, bitmap.width * bitmap.height * bitmap.channels);

    Bitmap *front = find_bitmap(global_assets, "FRONT");
    platform_memory_copy(bitmap.memory, front->memory, front->width * front->height * front->channels);

    if (number < 11 || number == 13)
        add_balls_to_bitmap(bitmap, circle_bitmap, number);

    Bitmap *front_bit = 0;
    switch(number) {
        case  0: front_bit = find_bitmap(global_assets, "FRONT0");  break;
        case 11: front_bit = find_bitmap(global_assets, "FRONT11"); break;
        case 12: front_bit = find_bitmap(global_assets, "FRONT12"); break;
    }

    if (front_bit != 0) {
        Vector2_s32 center = {
            (bitmap.width / 2) - (front_bit->width / 2),
            (bitmap.height / 2) - (front_bit->height / 2),
        };
        copy_blend_bitmap(bitmap, *front_bit, center, { 255, 255, 0 });
    }

    if (number == 13)
        number = -5;

    char str[3] = {};
    switch(number) {
        case -5: str[0] = '-'; str[1] = '5'; break;
        case 10: str[0] = '1'; str[1] = '0'; break;
        case 11: str[0] = '1'; str[1] = '1'; break;
        case 12: str[0] = '1'; str[1] = '2'; break;
        default: str[0] = number + 48; break;
    }

    Bitmap str_bitmap = create_string_into_bitmap(font, 300.0f, str);

    s32 padding = 60;
    //Vector3 text_color = ball_colors[number];
    Vector3 text_color = play_nine_green.xyz;
    if (number == -5)
        text_color = { 255, 0, 0 };

    Vector2_s32 bottom_right = {
        bitmap.width - str_bitmap.width - padding,
        bitmap.height - str_bitmap.height - padding
    };
    copy_blend_bitmap(bitmap, str_bitmap, { padding, padding }, text_color);
    copy_blend_bitmap(bitmap, str_bitmap, bottom_right, text_color);

    render_create_texture(&bitmap, TEXTURE_PARAMETERS_CHAR);

    platform_free(str_bitmap.memory);
    //platform_free(bitmap.memory);
  
    return bitmap;
}

internal void
init_card_bitmaps(Bitmap *bitmaps, Font *font) {
    Bitmap circle_bitmap = create_circle_bitmap({ 175, 175 });

    for (s32 i = 0; i <= 13; i++) {
        bitmaps[i] = create_card_bitmap(font, i, circle_bitmap);
    }

    platform_free(circle_bitmap.memory);
}

internal void
draw_card_bitmaps(Bitmap bitmaps[14], Vector2_s32 window_dim) {
    Vector2 dim = { window_dim.x / 14.0f, 0 };
    float32 percent = dim.x / bitmaps[0].width;
    dim.y = bitmaps[0].height * percent;
    //Vector2 pos = { 0, window_dim.y - dim.y };
    Vector2 pos = { 0, 0 };
    for (u32 i = 0; i < 14; i++) {
        pos.x = i * dim.x;
        draw_rect(pos, 0, dim, &bitmaps[i]);
    }
}
