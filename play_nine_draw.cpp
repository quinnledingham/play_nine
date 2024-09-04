internal float32
process_rotation(Rotation *rot, float32 seconds) {
    if (!rot->rotating)
        return rot->dest_degrees;

    if (rot->speed > 0.0f) {
        if (rot->degrees < rot->dest_degrees) {
            rot->degrees += (seconds * rot->speed);

            if (rot->degrees > rot->dest_degrees + 10.0f)
                rot->signal = false;

            if (rot->degrees > rot->dest_degrees)
                rot->degrees = rot->dest_degrees;
        } else {
            rot->rotating = false;
        }
    } else if (rot->speed < 0.0f) { 
        if (rot->degrees > rot->dest_degrees) {
            rot->degrees += (seconds * rot->speed);

            if (rot->degrees < rot->dest_degrees + 10.0f)
                rot->signal = false;

            if (rot->degrees < rot->dest_degrees)
                rot->degrees = rot->dest_degrees;
        } else {
            rot->rotating = false;
        }
    }

    return rot->degrees;
}

internal float32
get_draw_radius(u32 num_of_players, float32 hand_width, float32 card_height) {
    if (num_of_players == 2) {
        return 2.0f * card_height;
    } 

    float32 angle = (360.0f / float32(num_of_players)) / 2.0f;
    float32 radius = (hand_width / 2.0f) / tanf(angle * DEG2RAD);
    radius += card_height + 0.1f;

    if (num_of_players == 3) {
        radius += 1.0f;
    }

    return radius;
}

//
// Loading Model Matrices
//

internal void
rotate_coords(Vector3 *coords, float32 rad) {
    *coords = { 
         cosf(rad) * coords->x + sinf(rad) * coords->z, 
         coords->y, 
        -sinf(rad) * coords->x + cosf(rad) * coords->z 
    };
}

// -deg: negative because I want to go counter clockwise but still
// have degrees between players be positive
internal Vector3
get_hand_position(float32 hyp, float32 deg, u32 i) {
    float32 rad = -deg * DEG2RAD;
    Vector3 position = { 0, 0, 0 };
    position.x = hyp * cosf(i * rad);
    position.z = hyp * sinf(i * rad);
    return position;
}

internal float32
get_pile_y_scale(u32 cards) {
    float32 max_y_scale = 10.0f;
    float32 min_y_scale = 0.5f;
    float32 percent = float32(cards) / float32(DECK_SIZE);
    return (max_y_scale * percent) + min_y_scale;
}

internal Matrix_4x4
load_card_model(Vector3 position, float32 y_axis_rad, float32 z_axis_rad, Vector3 scale) {
    Quaternion rotation = get_rotation(y_axis_rad, Y_AXIS);
    rotation = get_rotation(z_axis_rad, Z_AXIS) * rotation;

    if (z_axis_rad != 0) {
        position.y += (CARD_MODEL_HEIGHT * scale.y);
    }
    
    return create_transform_m4x4(position, rotation, scale);
}

internal Vector3
get_card_position(Vector3 hand_position, u32 card_index, float32 y_axis_rad) {
    Vector3 card_position = {};
    card_position.x = hand_coords[card_index].x;
    card_position.z = hand_coords[card_index].y;
    rotate_coords(&card_position, y_axis_rad);
    card_position += hand_position;
    return card_position;
}

internal Vector3
get_card_position(Game_Draw_Info *info, u32 player_index, u32 card_index) {
    float32 y_axis_rad = info->player_hand_rads[player_index];
    Vector3 hand_position = info->hand_positions[player_index];
    return get_card_position(hand_position, card_index, y_axis_rad);
}

internal Matrix_4x4
load_player_card_model(u32 card_index, Vector3 hand_position, float32 y_axis_rad, float32 z_axis_rad, Vector3 scale) {
    Vector3 card_position = get_card_position(hand_position, card_index, y_axis_rad);
    return load_card_model(card_position, y_axis_rad, z_axis_rad, scale);
}

internal void
load_player_hand_models(Game_Draw *draw, u32 player_index, bool8 flipped[HAND_SIZE]) {
    Game_Draw_Info *info = &draw->info;
    float32 rad = info->player_hand_rads[player_index];
    Vector3 position = info->hand_positions[player_index];
    
    // draw player cards    
    for (u32 card_index = 0; card_index < 8; card_index++) {
        float32 z_axis_rad;
        if (flipped[card_index])
            z_axis_rad = 0.0f;
        else
            z_axis_rad = 180.0f * DEG2RAD;
        draw->hand_models[player_index][card_index] = load_player_card_model(card_index, position, rad, z_axis_rad, info->card_scale);
    }
}

internal void
load_player_card_models(Game *game, Game_Draw *draw) {
    for (u32 i = 0; i < game->num_of_players; i++) {
        load_player_hand_models(draw, i, game->players[i].flipped);
    }  
}

internal float32
linear_interpolate(float32 start, float32 end, float32 percent) {
    return start + ((end - start) * percent);
}

internal Vector3
linear_interpolate(Vector3 start, Vector3 end, float32 percent) {
    Vector3 result = {};
    result.x = linear_interpolate(start.x, end.x, percent);
    result.y = linear_interpolate(start.y, end.y, percent);
    result.z = linear_interpolate(start.z, end.z, percent);
    return result;
}

internal void
load_pile_card_models(Game *game, Game_Draw *draw, float32 rotation_degrees, float64 frame_time_s) {
    Game_Draw_Info *info = &draw->info;
    
    // move piles closer to player
    float32 center_of_piles_z = -info->x_hand_position + info->pile_distance_from_hand;

    info->pile_coords = {};
    info->pile_coords.x = -1.1f;
    info->pile_coords.z = center_of_piles_z;

    info->discard_pile_coords = {};
    info->discard_pile_coords.x = 1.1f;
    info->discard_pile_coords.z = center_of_piles_z;

    info->selected_card_coords = {};
    info->selected_card_coords.y = 1.0f;
    info->selected_card_coords.z = center_of_piles_z + -2.7f;

    float32 rad = info->player_hand_rads[game->active_player]; 
    float32 rotation_rads = rad - (rotation_degrees * DEG2RAD);
    info->pile_rads = rotation_rads;
    
    rotate_coords(&info->pile_coords, rotation_rads);
    rotate_coords(&info->discard_pile_coords, rotation_rads);
    rotate_coords(&info->selected_card_coords, rad);
    
    // draw card selected from pile
    if (game->turn_stage == SELECT_CARD) {
        draw->selected_card_model = load_card_model(info->selected_card_coords, rad, rotation_degrees * DEG2RAD, info->card_scale);
    }

    // pile
    {
        float32 pile_y_scale = get_pile_y_scale(DECK_SIZE - game->top_of_pile);
        Vector3 pile_scale = {1.0f, pile_y_scale, 1.0f};
        draw->top_of_pile_model = load_card_model(info->pile_coords, rotation_rads, 180.0f * DEG2RAD, pile_scale);

        info->pile_coords.y += (CARD_MODEL_HEIGHT * pile_y_scale); // used in do_draw_signals for animation
    }

    // discard pile
    if (game->top_of_discard_pile != 0) {
        float32 discard_pile_y_scale = get_pile_y_scale(game->top_of_discard_pile);
        Vector3 discard_pile_scale = {1.0f, discard_pile_y_scale, 1.0f};
        draw->top_of_discard_pile_model = load_card_model(info->discard_pile_coords, rotation_rads, 0.0f, discard_pile_scale);
        
        info->discard_pile_coords.y += (CARD_MODEL_HEIGHT * discard_pile_y_scale);
    }


}

internal void
load_game_draw_info(Game *game, Game_Draw_Info *info) {
    u32 draw_num_of_players = game->num_of_players;
    if (draw_num_of_players == 1)
        draw_num_of_players = 2;
    info->degrees_between_players = 360.0f / float32(draw_num_of_players);
    info->radius = get_draw_radius(draw_num_of_players, hand_width, 3.2f);
    
    for (u32 player_index = 0; player_index < game->num_of_players; player_index++) {
        float32 degrees = (info->degrees_between_players * player_index) - 90.0f;
        info->player_hand_rads[player_index] = degrees * DEG2RAD; 
        info->hand_positions[player_index] = get_hand_position(info->radius, info->degrees_between_players, player_index);

        if (player_index == 0)
            info->x_hand_position = info->hand_positions[player_index].x;
    }
}

//
// Drawing
//

void draw_card_model(Model *model, Descriptor color_set, Texture_Array *tex_array, s32 front_index, Matrix_4x4 model_matrix, bool8 flipped) {
    u32 draw[3] = { 1, 0, 2 };
    u32 flipped_draw[3] = { 2, 0, 1 };

    for (u32 i = 0; i < model->meshes_count; i++) {
        u32 draw_index = draw[i];
        if (flipped)
            draw_index = flipped_draw[i];

        Object object = {};
        object.model = model_matrix;
        object.index = tex_array->indices[14];

        switch(draw_index) {
            case 0: { 
                gfx_bind_shader("COLOR3D");
                render_bind_descriptor_set(color_set);
            } break;

            case 1:
                object.index = front_index;
            case 2: 
                gfx_bind_shader("BASIC3D");
                render_bind_descriptor_set(tex_array->desc);
            break;
        }

        render_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
        render_draw_mesh(&model->meshes[draw_index]);
    }
}

// drawing highlight
void draw_highlight(Model *model, Vector4 color, Matrix_4x4 model_matrix) {
    gfx_bind_shader("COLOR3D");

    Descriptor color_set = render_get_descriptor_set(5);
    render_update_ubo(color_set, (void *)&color);
    render_bind_descriptor_set(color_set);

    render_push_constants(SHADER_STAGE_VERTEX, (void *)&model_matrix, sizeof(Matrix_4x4));

    for (u32 i = 0; i < model->meshes_count; i++) {
        render_draw_mesh(&model->meshes[i]);
    }
}

internal void
draw_card(Model *card_model, Descriptor color_set, Texture_Array *tex_array, u32 number, Matrix_4x4 model, bool8 highlight, Vector4 highlight_color, bool8 flipped) {
    u32 bitmap_index = number;
    if (number == -5)
        bitmap_index = 13;

    if (highlight) {
        // hover card
        if (highlight_color == highlight_colors[1])
            model.F[13] += 0.05f;
        
        Matrix_4x4 model_scale = m4x4_scale(model, { 1.06f, 1.06f, 1.06f });
        render_bind_descriptor_set(light_set_2);
        draw_highlight(card_model, highlight_color, model_scale);
    }

    render_bind_descriptor_set(light_set);
    draw_card_model(card_model, color_set, tex_array, tex_array->indices[bitmap_index], model, flipped);
} 

internal Vector4
get_highlight_color(Game_Draw *draw, u32 index) {
    Vector4 highlight_color = highlight_colors[0];
    if (draw->highlight_hover[index])
        highlight_color = highlight_colors[1];
    if (draw->highlight_pressed[index])
        highlight_color = highlight_colors[2];
    return highlight_color;
}

internal void
draw_cards(Game *game, Game_Draw *draw, Model *card_model, bool8 highlight) {
    Texture_Array *tex_array = &draw->info.texture_array;
    Player *active_player = &game->players[game->active_player];
    enum Turn_Stages stage = game->turn_stage;

    gfx_bind_shader("COLOR3D");
    Descriptor color_set = render_get_descriptor_set(5);
    Vector4 color = { 150, 150, 150, 1 };
    render_update_ubo(color_set, &color);

    {
        bool8 h = stage == SELECT_PILE && highlight;
        draw_card(card_model, color_set, tex_array, deck[game->pile[game->top_of_pile]], draw->top_of_pile_model, h, get_highlight_color(draw, GI_PICKUP_PILE), false);
    }

    if (game->top_of_discard_pile != 0 && !draw->moving_card) {
        Matrix_4x4 model = draw->top_of_discard_pile_model;
        s32 card = deck[game->discard_pile[game->top_of_discard_pile - 1]];
        bool8 h = ((stage == SELECT_CARD && game->pile_card) || stage == SELECT_PILE) && highlight;
        draw_card(card_model, color_set, tex_array, card, model, h, get_highlight_color(draw, GI_DISCARD_PILE), true);
    }

    if (draw->moving_card && game->top_of_discard_pile > 1) {
        s32 card = deck[game->discard_pile[game->top_of_discard_pile - 2]];
        bool8 h = ((stage == SELECT_CARD && game->pile_card) || stage == SELECT_PILE) && highlight;
        draw_card(card_model, color_set, tex_array, card, draw->top_of_discard_pile_model, h, get_highlight_color(draw, GI_DISCARD_PILE), true);
    }
    
    for (u32 player_index = 0; player_index < game->num_of_players; player_index++) {
        for (u32 card_index = 0; card_index < HAND_SIZE; card_index++) {
            bool8 h = stage != SELECT_PILE && player_index == game->active_player && highlight;
            if (stage == FLIP_CARD && active_player->flipped[card_index]) h = false;

            s32 card = deck[game->players[player_index].cards[card_index]];
            draw_card(card_model, color_set, tex_array, card, draw->hand_models[player_index][card_index], h, get_highlight_color(draw, card_index), game->players[player_index].flipped[card_index]);
        }
    }

    if (game->top_of_discard_pile != 0 && draw->moving_card) {
        Matrix_4x4 model = draw->moving_card_model;
        s32 card = deck[game->discard_pile[game->top_of_discard_pile - 1]];
        bool8 h = ((stage == SELECT_CARD && game->pile_card) || stage == SELECT_PILE) && highlight;
        draw_card(card_model, color_set, tex_array, card, model, h, get_highlight_color(draw, GI_DISCARD_PILE), true);
    }
    
    // always draw new card on top
    if (stage == SELECT_CARD) {
        s32 card = deck[game->new_card];
        draw_card(card_model, color_set, tex_array, card, draw->selected_card_model, false, play_nine_yellow, true);
    }
}

//
// Name Plates
// 

internal void
load_name_plates(Game *game, Game_Draw *draw) {
    for (u32 i = 0; i < game->num_of_players; i++) {
        if (draw->name_plates[i].gpu_info != 0) {
            platform_free(draw->name_plates[i].memory);
            render_delete_texture(&draw->name_plates[i]);
        }

        Bitmap string_bitmap = create_string_into_bitmap(default_font, 350.0f, game->players[i].name);
        if (!game->players[i].is_bot)
            draw->name_plates[i] = string_bitmap;
        else {
            Bitmap bitmap {};
            bitmap.width = draw->bot_bitmap->width + string_bitmap.width;
            bitmap.height = string_bitmap.height;
            bitmap.channels = 1;
            bitmap.pitch = bitmap.width * bitmap.channels;
            bitmap.memory = (u8*)platform_malloc(bitmap.width * bitmap.height * bitmap.channels);
            memset(bitmap.memory, 0x00, bitmap.width * bitmap.height * bitmap.channels);
            copy_blend_bitmap(bitmap, *draw->bot_bitmap, { 0, 0 }, { 255, 255, 255 });
            copy_blend_bitmap(bitmap, string_bitmap, { draw->bot_bitmap->width, 0 }, { 255, 255, 255 });
            draw->name_plates[i] = bitmap;
            platform_free(string_bitmap.memory);
        }
        if (draw->name_plates[i].height != 0) // height because bot bitmap is set with string height and string height can be zero
            render_create_texture(&draw->name_plates[i], TEXTURE_PARAMETERS_CHAR);
    }

}

internal void
unload_name_plate(Game_Draw *draw, u32 index) {
    render_delete_texture(&draw->name_plates[index]);
    
    u32 dest_index = index;
    u32 src_index = index + 1;
    for (src_index; src_index < MAX_PLAYERS; src_index++) {
        draw->name_plates[dest_index++] = draw->name_plates[src_index];
    }
}

internal void
unload_name_plates(Game_Draw *draw) {
    for (u32 i = 0; i < MAX_PLAYERS; i++) {
        render_delete_texture(&draw->name_plates[i]);
    }
}

internal void
draw_name_plates(Game *game, Game_Draw *draw) {
    Descriptor color_set_2 = render_get_descriptor_set(4);

    for (u32 i = 0; i < game->num_of_players; i++) {
        if (draw->name_plates[i].gpu_info == 0)
            continue;

        float32 degrees = (draw->info.degrees_between_players * i) - 90.0f;
        float32 rad = degrees * DEG2RAD; 
        Vector4 name_plates_color = { 255, 255, 255, 1 };
        Object object = {};

        Descriptor bitmap_desc = render_get_descriptor_set(2);
        object.index = render_set_bitmap(&bitmap_desc, &draw->name_plates[i]);
        render_bind_descriptor_set(bitmap_desc);

        render_bind_descriptor_sets(color_set_2, &name_plates_color);

        Vector3 position = get_hand_position(draw->info.radius, draw->info.degrees_between_players, i);
        position += normalized(position) * 4.1f;

        Vector3 scale = {(float32)draw->name_plates[i].width / 250.0f, (float32)draw->name_plates[i].height / 250.0f, 1.0f};
        position.y = 0.1f;

        Quaternion rot = get_rotation(-90.0f * DEG2RAD, { 1, 0, 0 });
        rot = rot * get_rotation(rad, { 0, 1, 0 });
        object.model = create_transform_m4x4(position, rot, scale);
        render_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));

        render_draw_mesh(&shapes.rect_3D_mesh);
    }
}

internal void
draw_triangle_indicator(Game *game, Game_Draw *draw) {    
    float32 indicator_distance_from_pile = -1.9f;
    
    Vector3 triangle_coords = {};
    triangle_coords.y = 0.01f;
    triangle_coords.z = -draw->info.x_hand_position + draw->info.pile_distance_from_hand + indicator_distance_from_pile;
    float32 degrees = (draw->info.degrees_between_players * game->active_player) - 90.0f;
    float32 rads = (degrees - draw->camera_rotation.degrees) * DEG2RAD;
    rotate_coords(&triangle_coords, rads);
    rads -= 135.0f * DEG2RAD; // to align the triangle mesh the way it is set up (bottom left corner)
    draw_triangle(triangle_coords, { -90.0f * DEG2RAD, rads, 0 }, { 0.5f, 0.5f, 1 }, { 255, 255, 255, 1 });
}

//      180
// 270        90
//       0
internal void
draw_game(State *state, Assets *assets, Shader *shader, Game *game) {
    Texture_Array *tex_array = &state->game_draw.info.texture_array;
    render_depth_test(true);
    
    gfx_bind_shader("BASIC3D");    
    render_bind_descriptor_set(state->scene_set);
    render_bind_descriptor_set(light_set);
    
    // Skybox
    draw_cube({ 0, 0, 0 }, 0.0f, { 100, 100, 100 }, { 30, 20, 10, 1 });

    // Table
    gfx_bind_shader("BASIC3D");
    //render_bind_pipeline(&texture_shader->pipeline);
    render_bind_descriptor_set(tex_array->desc);

    Object object = {};
    object.model = create_transform_m4x4({ 0, -0.1f - 0.1f, 0 }, get_rotation(0, { 0, 1, 0 }), {15.6f, 2.0f, 15.6f});
    object.index = tex_array->indices[15];
    render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

    Model *model = find_model(assets, "TABLE");
    for (u32 i = 0; i < model->meshes_count; i++) { 
        render_draw_mesh(&model->meshes[i]);
    }

    //Shader *text_shader = find_shader(global_assets, "TEXT3D");
    //render_bind_pipeline(&text_shader->pipeline);
    gfx_bind_shader("TEXT3D");

    // Name Plate
    draw_name_plates(game, &state->game_draw);

    render_depth_test(true);

    draw_triangle_indicator(&state->game, &state->game_draw);

    // Cards
    gfx_bind_shader("BASIC3D");
    //render_bind_pipeline(&texture_shader->pipeline);
    render_bind_descriptor_set(tex_array->desc);

    Model *card_model = find_model(assets, "CARD");

    render_depth_test(false);
    bool8 highlight = state->is_active && state->game.round_type != HOLE_OVER && !state->game_draw.camera_rotation.rotating;
    
    if (state->game.round_type == HOLE_OVER || !highlight)
        render_depth_test(true);
    
    draw_cards(game, &state->game_draw, card_model, highlight);

    render_depth_test(false);
}

// Hole 11
// buffer must be 8 bytes
internal void
get_hole_text(char *buffer, u32 hole, u32 length) {
    platform_memory_set(buffer, 0, 16);
    platform_memory_copy(buffer, (void*)"Hole ", 5);
    buffer += 5;
    buffer += s32_to_char_array(buffer, 3, hole);
    platform_memory_copy(buffer, (void*)" / ", 3);
    buffer += 3;
    s32_to_char_array(buffer, 3, length);
}

internal void
draw_timer(float32 time, Vector2_s32 window_dim) {
    if (time > 7.0f) {
        float32 pixel_height = 40.0f;
        float32 time_left = 10.0f - time;
        
        char buffer[20];
        float_to_char_array(time_left, buffer, 20);
        
        String_Draw_Info info = get_string_draw_info(default_font, buffer, -1, pixel_height);
        float32 x_coords = ((float32)window_dim.width / 2.0f) - (info.dim.width / 2.0f);
        draw_string_tl(default_font, buffer, { x_coords, 40.0f }, pixel_height, { 255, 255, 255, 1 });
    }
}

internal void
draw_game_hud(State *state, Vector2_s32 window_dim, App_Input *input, bool8 full_menu) {
    // depth test already off from draw_game()
    render_bind_descriptor_set(state->scene_ortho_set);

    Font *font = find_font(&state->assets, "CASLON");
    float32 hole_pixel_height = window_dim.x / 30.0f;
    float32 pixel_height = window_dim.x / 20.0f;
    float32 padding = 10.0f;

    char hole_text[16];
    u32 hole_number = state->game.holes_played + 1;
    if (state->game.round_type == HOLE_OVER)
        hole_number -= 1;
    get_hole_text(hole_text, hole_number, state->game.holes_length);

    const char *round_type_text = round_types[state->game.round_type];
    if (state->game.game_over) {
        round_type_text = "Game Over";
    }
    
    String_Draw_Info hole_string_info = get_string_draw_info(font, hole_text, -1, hole_pixel_height);
    String_Draw_Info string_info = get_string_draw_info(font, round_type_text, -1, pixel_height);

    Vector2 hole_coords = { padding, padding };
    Vector2 round_coords = hole_coords + Vector2{ 0.0f, 2.0f + hole_string_info.dim.y };

    draw_string_tl(font, hole_text, hole_coords, hole_pixel_height, { 255, 255, 255, 1 });
    draw_string_tl(font, round_type_text, round_coords, pixel_height, { 255, 255, 255, 1 });

    gui.start();
    gui.rect.coords = { 0, 0 };
    gui.rect.dim = cv2(window_dim);
    gui.input = {
        &input->active,
    
        &state->controller.select,
        &state->controller.left,
        &state->controller.forward,
        &state->controller.right,
        &state->controller.backward,
    
        &input->mouse,
        &state->controller.mouse_left
    };
    gui.handle_input = true;

    float32 button_width = window_dim.x / 6.0f;
    Player *active_player = &state->game.players[state->game.active_player];

    if (state->menu_list.mode == PAUSE_MENU) {

        draw_pause_menu(state, &state->menu_list.menus[PAUSE_MENU], full_menu, window_dim);

    } else if (state->game.round_type == HOLE_OVER) {

        Vector2 dim = { button_width, pixel_height };
        Vector2 scoreboard_coords = { gui.rect.dim.x - dim.x - padding, gui.rect.dim.y - dim.y - padding };
        Vector2 next_coords = scoreboard_coords;
        next_coords.y -= dim.y + padding;
        Vector2 lobby_coords = next_coords;
        lobby_coords.y -= dim.y + padding;

        if (full_menu) {
            const char *play_button_text;
            if (!state->game.game_over) {
                play_button_text = "Next Hole";
            } else {
                play_button_text = "Play Again";

                Player *winner = get_winner(&state->game);
                char buffer[MAX_NAME_SIZE + 6];
                platform_memory_set(buffer, 0, MAX_NAME_SIZE + 6);
                if (winner) {
                    const char *wins_string = " Wins!";
                    u32 name_length = get_length(winner->name);
                    platform_memory_copy(buffer, winner, name_length);
                    platform_memory_copy(buffer + name_length, (void *)wins_string, 6);
                } else {
                    const char *tie_string = "Tie";
                    platform_memory_copy(buffer, tie_string, 3);
                }
                
                String_Draw_Info string_info = get_string_draw_info(font, buffer, -1, pixel_height);
                Vector2 winner_coords = { 0, 10 };
                winner_coords.x = center(float32(window_dim.x), string_info.dim.x);
                draw_string_tl(font, buffer, winner_coords, pixel_height, { 255, 255, 255, 1 });
                
                if (gui_button(&gui, default_style, "Lobby", lobby_coords, dim)) {
                    state->menu_list.previous_mode = PAUSE_MENU;
                    state->menu_list.mode = LOCAL_MENU;

                    if (state->mode == MODE_SERVER)
                        server_send_menu_mode(state->menu_list.mode);
                }
            }
            
            if (gui_button(&gui, default_style, play_button_text, next_coords, dim)) {
                state->menu_list.mode = IN_GAME;
                if (!state->game.game_over)
                    start_hole(&state->game);
                else
                    start_game(&state->game, state->game.num_of_players);
                    
                add_draw_signal(draw_signals, SIGNAL_ALL_PLAYER_CARDS);    
                if (state->mode == MODE_SERVER) {
                    server_send_game(&state->game, draw_signals, DRAW_SIGNALS_AMOUNT);
                    server_send_menu_mode(state->menu_list.mode);
                }
                
            }
        }
        
        if (gui_button(&gui, default_style, "Scoreboard", scoreboard_coords, dim)) {
            state->menu_list.mode = SCOREBOARD_MENU;
        }

    } else if (state->game.turn_stage == FLIP_CARD && get_number_flipped(active_player->flipped) == HAND_SIZE - 1) {
    
        if (state->is_active) {
            Vector2 dim = { button_width, pixel_height };
            Vector2 coords = { window_dim.x - dim.x - padding, window_dim.y - dim.y - padding };
            if (gui_button(&gui, default_style, "Pass", coords, dim)) {
                state->pass_selected = true; // in update_game feed this into selected
            }
        }

    }

    gui_do_keyboard_input(&gui);
    gui.end();

    draw_timer(state->game.turn_time, window_dim);
}

//
// Card Animations
//

internal void
add_card_animation(Game_Draw *draw, Card_Animation animation) {
    for (u32 i = 0; i < draw->max_card_animations; i++) {
        if (!draw->animations[i].in_use) {
            draw->animations[i] = animation;
            draw->animations[i].in_use = true;
            draw->animations_count++;
            return;
        }
    }

    logprint("add_card_animation()", "too many animations\n");
}

internal void
do_card_animation(Game_Draw *draw, Card_Animation *animation, float32 frame_time_s) {
    if (animation->moving) {
        for (u32 i = 0; i < animation->keyframes_count; i++) {
            Card_Animation_Keyframe *keyframe = &animation->keyframes[i];
            if (keyframe->time_elapsed < keyframe->time_duration) {
                if (keyframe->dynamic) {
                    keyframe->dest = *keyframe->dynamic_dest;
                }
                
                keyframe->time_elapsed += frame_time_s;
                
                float32 percent = keyframe->time_elapsed / keyframe->time_duration;
                animation->position = linear_interpolate(keyframe->start, keyframe->dest, percent);

                if (percent >= 1.0f && i == animation->keyframes_count - 1) {
                    animation->moving = false;
                    animation->position = keyframe->dest;
                }

                break;
            }
        }
    }

    if (animation->rotation.rotating) {
        float32 z_axis_degrees = process_rotation(&animation->rotation, frame_time_s);
        animation->z_axis_rad = z_axis_degrees * DEG2RAD;
    }

    if (animation->dynamic) {
        animation->y_axis_rad = *animation->dynamic_y_axis_rad;
    }
    
    *animation->model = load_card_model(animation->position, animation->y_axis_rad, animation->z_axis_rad, draw->info.card_scale);

    if (!animation->rotation.rotating && !animation->moving) {
        animation->in_use = false;
        draw->animations_count--;

        if (animation->model == &draw->moving_card_model)
            draw->moving_card = false;
    }
}

internal void
do_card_animations(Game_Draw *draw, float32 frame_time_s) {
    for (u32 i = 0; i < draw->max_card_animations; i++) {
        if (!draw->animations[i].in_use) {
            continue;
        }

        do_card_animation(draw, &draw->animations[i], frame_time_s);
    }
}

//
// Draw_Signals
//

internal void
add_draw_signal_to_signals(Draw_Signal *signals, Draw_Signal signal) {
    for (u32 i = 0 ; i < DRAW_SIGNALS_AMOUNT; i++) {
        if (!signals[i].in_use) {
            signals[i] = signal;
            //if (*global_mode == MODE_SERVER)
            //    server_add_draw_signal(signals[i]);
            return;
        }
    }

    logprint("add_draw_signal()", "ran out of signal places\n");
}

internal void
add_draw_signal(Draw_Signal *signals, u32 in_type, u32 in_card_index, u32 in_player_index, bool8 flipped) {
    Draw_Signal signal = Draw_Signal(in_type, in_card_index, in_player_index);
    signal.flipped = flipped;
    add_draw_signal_to_signals(signals, signal);
}

internal void
add_draw_signal(Draw_Signal *signals, u32 in_type, u32 in_card_index, u32 in_player_index) {
    Draw_Signal signal = Draw_Signal(in_type, in_card_index, in_player_index);
    add_draw_signal_to_signals(signals, signal);
}

internal void
add_draw_signal(Draw_Signal *signals, u32 in_type) {
    add_draw_signal(signals, in_type, 0, 0);
}

internal void
add_draw_signal(Draw_Signal *signals, Draw_Signal s) {
    add_draw_signal_to_signals(signals, s);
}

internal void
do_draw_signals(Draw_Signal *signals, Game *game, Game_Draw *draw) {         
    Game_Draw_Info *info = &draw->info;
    
    for (u32 i = 0; i < DRAW_SIGNALS_AMOUNT; i++) {
        if (!signals[i].in_use)
            continue;
        signals[i].in_use = false;    
        
        switch(signals[i].type) {
            case SIGNAL_NEXT_PLAYER_ROTATION: {
                draw->camera_rotation = {
                    true,
                    true,
                    draw->info.degrees_between_players,
                    0.0f,
                    -draw->info.rotation_speed
                };
                play_sound("WOOSH");

            } break;
            
            case SIGNAL_ALL_PLAYER_CARDS: {
                load_game_draw_info(game, &draw->info);
                load_player_card_models(game, draw); 
            } break;
            
            case SIGNAL_FLIP_CARD: {
                Card_Animation animation = {};
                Vector3 position = get_card_position(&draw->info, signals[i].player_index, signals[i].card_index);
                Vector3 middle = position;
                middle.y = 1.0f;
                animation.model = &draw->hand_models[signals[i].player_index][signals[i].card_index];
                animation.y_axis_rad = draw->info.player_hand_rads[signals[i].player_index];
                animation.add_movement(position, middle, position, 0.5f); 
                animation.add_rotation(180.0f, 0.0f, -draw->info.card_flipping_speed);
                add_card_animation(draw, animation);
                play_sound("TAP");
            } break;

            case SIGNAL_REPLACE: {
                Vector3 position = get_card_position(&draw->info, signals[i].player_index, signals[i].card_index);
                draw->moving_card = true;
                
                Card_Animation animation = {};
                Vector3 above_position = position;
                above_position.y += 2.0f;
                animation.model = &draw->moving_card_model;
                animation.dynamic = true;
                animation.dynamic_y_axis_rad = &info->pile_rads;
                animation.z_axis_rad = 0.0f;
                animation.add_movement(position, above_position, &info->discard_pile_coords, 0.5f); 
                if (!signals[i].flipped) {
                    animation.add_rotation(180.0f, 0.0f, -draw->info.card_flipping_speed);
                }
                add_card_animation(draw, animation);

                Card_Animation in_animation = {};
                in_animation.model = &draw->hand_models[signals[i].player_index][signals[i].card_index];
                in_animation.y_axis_rad = draw->info.player_hand_rads[signals[i].player_index];
                in_animation.z_axis_rad = 0.0f;
                in_animation.add_movement(info->selected_card_coords, position, 0.25f); 
                add_card_animation(draw, in_animation);
            } break;

            case SIGNAL_DISCARD_SELECTED: {
                draw->moving_card = true;
                    
                Card_Animation animation = {};
                animation.model = &draw->moving_card_model;
                animation.y_axis_rad = draw->info.player_hand_rads[game->active_player];
                animation.z_axis_rad = 0.0f;
                animation.add_movement(info->selected_card_coords, info->discard_pile_coords, 0.25f); 
                add_card_animation(draw, animation);
            } break;
            
            case SIGNAL_NEW_CARD_FROM_PILE: {
                Vector3 above_pile = info->pile_coords;
                above_pile.y += 1.0f;
                
                Card_Animation animation = {};
                animation.model = &draw->selected_card_model;
                animation.y_axis_rad = draw->info.player_hand_rads[game->active_player];
                animation.add_movement(info->pile_coords, above_pile, info->selected_card_coords, 0.5f); 
                animation.add_rotation(180.0f, 0.0f, -draw->info.card_flipping_speed);
                add_card_animation(draw, animation);
            } break;

            case SIGNAL_NEW_CARD_FROM_DISCARD: {
                Vector3 above_pile = info->discard_pile_coords;
                above_pile.y += 1.0f;
                
                Card_Animation animation = {};
                animation.model = &draw->selected_card_model;
                animation.y_axis_rad = draw->info.player_hand_rads[game->active_player];
                animation.add_movement(info->discard_pile_coords, above_pile, info->selected_card_coords, 0.25f); 
                add_card_animation(draw, animation);
            } break;

            case SIGNAL_NAME_PLATES: load_name_plates(game, draw); break;

            case SIGNAL_UNLOAD_NAME_PLATE: unload_name_plate(draw, signals[i].player_index); break;
        }
    }
}

// Loading Icon

void Loading_Icon::enable(Bitmap *in_bitmap) {
    enabled = true;
    bitmap = in_bitmap;
    rotation = 0.0f;
}

void Loading_Icon::disable() {
    enabled = false;
}

void Loading_Icon::draw(Vector2_s32 window_dim) {
    if (!enabled)
        return;

    rotation += 0.01f;

    float32 size = 100.0f;
    if (window_dim.y < window_dim.x)
        size = window_dim.y * 0.1f;
    else
        size = window_dim.x * 0.1f;
        
    Vector2 dim = { size, size };
    Vector2 coords = { window_dim.x - dim.x, window_dim.y - dim.y };
    draw_rect(coords, rotation, dim, bitmap);
}
