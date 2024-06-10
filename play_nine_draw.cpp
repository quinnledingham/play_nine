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
        position.y += (0.101767f * scale.y);
    }
    
    return create_transform_m4x4(position, rotation, scale);
}

internal Matrix_4x4
load_player_card_model(u32 card_index, Vector3 hand_position, float32 y_axis_rad, float32 z_axis_rad, Vector3 scale) {
    Vector3 card_position = {};
    card_position.x = hand_coords[card_index].x;
    card_position.z = hand_coords[card_index].y;
    rotate_coords(&card_position, y_axis_rad);
    card_position += hand_position;

    return load_card_model(card_position, y_axis_rad, z_axis_rad, scale);
}

internal void
flip_card_model(Game_Draw *draw, u32 player_index, u32 card_index, float32 x_axis_rad) {
    float32 degrees = (draw->degrees_between_players * player_index) - 90.0f;
    float32 rad = degrees * DEG2RAD; 
    Vector3 position = get_hand_position(draw->radius, draw->degrees_between_players, player_index);
    draw->hand_models[player_index][card_index] = load_player_card_model(card_index, position, rad, x_axis_rad, draw->card_scale);   
}

internal void
load_player_hand_models(Game_Draw *draw, u32 player_index) {
    float32 degrees = (draw->degrees_between_players * player_index) - 90.0f;
    float32 rad = degrees * DEG2RAD; 
    Vector3 position = get_hand_position(draw->radius, draw->degrees_between_players, player_index);
    
    if (player_index == 0)
        draw->x_hand_position = position.x;

    // draw player cards    
    for (u32 card_index = 0; card_index < 8; card_index++) {
        draw->hand_models[player_index][card_index] = load_player_card_model(card_index, position, rad, 180.0f * DEG2RAD, draw->card_scale);
    }
}

internal void
load_player_card_models(Game *game, Game_Draw *draw) {
    for (u32 i = 0; i < game->num_of_players; i++) {
        load_player_hand_models(draw, i);
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
    // move piles closer to player
    float32 center_of_piles_z = -draw->x_hand_position + draw->pile_distance_from_hand;

    Vector3 pile_coords = {};
    pile_coords.x = -1.1f;
    pile_coords.z = center_of_piles_z;

    Vector3 discard_pile_coords = {};
    discard_pile_coords.x = 1.1f;
    discard_pile_coords.z = center_of_piles_z;

    Vector3 selected_card_coords = {};
    selected_card_coords.y = 1.0f;
    selected_card_coords.z = center_of_piles_z + -2.7f;

    float32 degrees = (draw->degrees_between_players * game->active_player) - 90.0f;
    float32 rad = degrees * DEG2RAD; 
    float32 rotation_rads = (degrees - rotation_degrees) * DEG2RAD;
    
    rotate_coords(&pile_coords, rotation_rads);
    rotate_coords(&discard_pile_coords, rotation_rads);
    
    // draw card selected from pile
    if (game->turn_stage == SELECT_CARD) {
        rotate_coords(&selected_card_coords, rad);
        float32 rotation_degrees = process_rotation(&draw->new_card_rotation, (float32)frame_time_s);
        float32 percent_rotated = 1.0f - (rotation_degrees / 180.0f);
        selected_card_coords = linear_interpolate(pile_coords, selected_card_coords, percent_rotated);

        draw->new_card_model = load_card_model(selected_card_coords, rad, rotation_degrees * DEG2RAD, draw->card_scale);
    }

    // pile
    {
        float32 pile_y_scale = get_pile_y_scale(DECK_SIZE - game->top_of_pile);
        Vector3 pile_scale = {1.0f, pile_y_scale, 1.0f};
        draw->top_of_pile_model = load_card_model(pile_coords, rotation_rads, 180.0f * DEG2RAD, pile_scale);
    }

    // discard pile
    if (game->top_of_discard_pile != 0) {
        float32 discard_pile_y_scale = get_pile_y_scale(game->top_of_discard_pile);
        Vector3 discard_pile_scale = {1.0f, discard_pile_y_scale, 1.0f};
        draw->top_of_discard_pile_model = load_card_model(discard_pile_coords, rotation_rads, 0.0f, discard_pile_scale);
    }

}

internal void
do_card_animations(Game *game, Game_Draw *draw, float64 frame_time_s) {
    for (u32 player_index = 0; player_index < game->num_of_players; player_index++) {
        for (u32 card_index = 0; card_index < 8; card_index++) {
            Player *player = &game->players[player_index];
            if (draw->card_rotation[player_index][card_index].rotating) {
                float32 rotation_degrees = process_rotation(&draw->card_rotation[player_index][card_index], (float32)frame_time_s);
                flip_card_model(draw, player_index, card_index, rotation_degrees * DEG2RAD);
            }
        }
    }
}

//
// Drawing
//

void draw_card_model(Model *model, Descriptor color_set, s32 front_index, s32 back_index, Matrix_4x4 model_matrix, bool8 flipped) {
    u32 draw[3] = { 1, 0, 2 };
    u32 flipped_draw[3] = { 2, 0, 1 };

    for (u32 i = 0; i < model->meshes_count; i++) {
        u32 draw_index = draw[i];
        if (flipped)
            draw_index = flipped_draw[i];

        Object object = {};
        object.model = model_matrix;
        object.index = back_index;

        switch(draw_index) {
            case 0: 
                render_bind_pipeline(&pipelines[PIPELINE_3D_COLOR]); 
                render_bind_descriptor_set(color_set);
            break;

            case 1:
                object.index = front_index;
            case 2: 
                render_bind_pipeline(&pipelines[PIPELINE_3D_TEXTURE]); 
                render_bind_descriptor_set(texture_desc);
            break;
        }

        render_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
        render_draw_mesh(&model->meshes[draw_index]);
    }
}

// drawing highlight
void draw_highlight(Model *model, Render_Pipeline *color_pipeline, Vector4 color, Matrix_4x4 model_matrix) {
    render_bind_pipeline(&pipelines[PIPELINE_3D_COLOR]);

    Descriptor color_set = render_get_descriptor_set(&layouts[5]);
    render_update_ubo(color_set, (void *)&color);
    render_bind_descriptor_set(color_set);

    render_push_constants(SHADER_STAGE_VERTEX, (void *)&model_matrix, sizeof(Matrix_4x4));

    for (u32 i = 0; i < model->meshes_count; i++) {
        render_draw_mesh(&model->meshes[i]);
    }
}

internal void
draw_card(Model *card_model, Descriptor color_set, s32 indices[16], u32 number, Matrix_4x4 model, bool8 highlight, Vector4 highlight_color, bool8 flipped) {
    u32 bitmap_index = number;
    if (number == -5)
        bitmap_index = 13;

    if (highlight) {
        Matrix_4x4 model_scale = m4x4_scale(model, { 1.06f, 1.06f, 1.06f });
        render_bind_descriptor_set(light_set_2);
        draw_highlight(card_model, &pipelines[PIPELINE_3D_COLOR], highlight_color, model_scale);
    }

    render_bind_descriptor_set(light_set);
    draw_card_model(card_model, color_set, indices[bitmap_index], indices[14], model, flipped);
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
draw_cards(Game *game, Game_Draw *draw, Model *card_model, bool8 highlight, s32 indices[16]) {
    Player *active_player = &game->players[game->active_player];
    enum Turn_Stages stage = game->turn_stage;
    
    Descriptor color_set = render_get_descriptor_set(&layouts[5]);
    Vector4 color = { 150, 150, 150, 1 };
    render_update_ubo(color_set, &color);

    for (u32 player_index = 0; player_index < game->num_of_players; player_index++) {
        for (u32 card_index = 0; card_index < HAND_SIZE; card_index++) {
            bool8 h = stage != SELECT_PILE && player_index == game->active_player && highlight;
            if (stage == FLIP_CARD && active_player->flipped[card_index]) h = false;

            s32 card = deck[game->players[player_index].cards[card_index]];
            draw_card(card_model, color_set, indices, card, draw->hand_models[player_index][card_index], h, get_highlight_color(draw, card_index), game->players[player_index].flipped[card_index]);
        }
    }

    {
        bool8 h = stage == SELECT_PILE && highlight;
        draw_card(card_model, color_set, indices, deck[game->pile[game->top_of_pile]], draw->top_of_pile_model, h, get_highlight_color(draw, PICKUP_PILE), false);
    }

    if (game->top_of_discard_pile != 0) {
        bool8 h = ((stage == SELECT_CARD && game->pile_card) || stage == SELECT_PILE) && highlight;
        draw_card(card_model, color_set, indices, deck[game->discard_pile[game->top_of_discard_pile - 1]], draw->top_of_discard_pile_model, h, get_highlight_color(draw, DISCARD_PILE), true);
    }

    // always draw new card on top
    if (stage == SELECT_CARD) {
        s32 card = deck[game->new_card];
        draw_card(card_model, color_set, indices, card, draw->new_card_model, false, play_nine_yellow, true);
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

    draw->name_plates_loaded = true;
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
    for (u32 i = 0; i < game->num_of_players; i++) {
        if (draw->name_plates[i].gpu_info == 0)
            continue;

        float32 degrees = (draw->degrees_between_players * i) - 90.0f;
        float32 rad = degrees * DEG2RAD; 
        Vector4 name_plates_color = { 255, 255, 255, 1 };
        Object object = {};

        Descriptor bitmap_desc = render_get_descriptor_set(&layouts[2]);
        object.index = render_set_bitmap(&bitmap_desc, &draw->name_plates[i]);
        render_bind_descriptor_set(bitmap_desc);

        Descriptor color_set_2 = render_get_descriptor_set(&layouts[4]);
        render_update_ubo(color_set_2, (void *)&name_plates_color);
        render_bind_descriptor_set(color_set_2);

        Vector3 position = get_hand_position(draw->radius, draw->degrees_between_players, i);
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
    triangle_coords.z = -draw->x_hand_position + draw->pile_distance_from_hand + indicator_distance_from_pile;
    float32 degrees = (draw->degrees_between_players * game->active_player) - 90.0f;
    float32 rads = (degrees - draw->camera_rotation.degrees) * DEG2RAD;
    rotate_coords(&triangle_coords, rads);
    rads -= 135.0f * DEG2RAD; // to align the triangle mesh the way it is set up (bottom left corner)
    draw_triangle(triangle_coords, { -90.0f * DEG2RAD, rads, 0 }, { 0.5f, 0.5f, 1 }, { 255, 255, 255, 1 });
}

//      180
// 270        90
//       0
internal void
draw_game(State *state, Assets *assets, Shader *shader, Game *game, s32 indices[16]) {
    render_depth_test(true);
    render_bind_pipeline(&pipelines[PIPELINE_3D_TEXTURE]);
    render_bind_descriptor_set(state->scene_set);
    render_bind_descriptor_set(light_set);
    
    // Skybox
    draw_cube({ 0, 0, 0 }, 0.0f, { 100, 100, 100 }, { 30, 20, 10, 1 });

    // Table
    render_bind_pipeline(&pipelines[PIPELINE_3D_TEXTURE]);
    render_bind_descriptor_set(texture_desc);

    Object object = {};
    object.model = create_transform_m4x4({ 0, -0.1f - 0.1f, 0 }, get_rotation(0, { 0, 1, 0 }), {15.6f, 2.0f, 15.6f});
    object.index = indices[15];
    render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

    Model *model = find_model(assets, "TABLE");
    for (u32 i = 0; i < model->meshes_count; i++) { 
        render_draw_mesh(&model->meshes[i]);
    }

    render_bind_pipeline(&pipelines[PIPELINE_3D_TEXT]);
    render_bind_descriptor_set(light_set);

    // Name Plate
    draw_name_plates(game, &state->game_draw);

    render_depth_test(true);

    draw_triangle_indicator(&state->game, &state->game_draw);

    // Cards
    render_bind_pipeline(&pipelines[PIPELINE_3D_TEXTURE]);
    render_bind_descriptor_set(texture_desc);

    Model *card_model = find_model(assets, "CARD");

    render_depth_test(false);
    bool8 highlight = state->is_active && state->game.round_type != HOLE_OVER && !state->game_draw.camera_rotation.rotating;
    
    if (state->game.round_type == HOLE_OVER || !highlight)
        render_depth_test(true);
    
    draw_cards(game, &state->game_draw, card_model, highlight, indices);

    render_depth_test(false);
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
    for (u32 i = 0; i < DRAW_SIGNALS_AMOUNT; i++) {
        if (!signals[i].in_use)
            continue;
        
        switch(signals[i].type) {
            case SIGNAL_ALL_PLAYER_CARDS: {
                u32 draw_num_of_players = game->num_of_players;
                if (draw_num_of_players == 1)
                    draw_num_of_players = 2;
                draw->degrees_between_players = 360.0f / float32(draw_num_of_players);
                draw->radius = get_draw_radius(draw_num_of_players, hand_width, 3.2f);
                load_player_card_models(game, draw); 
            } break;
            
            case SIGNAL_ACTIVE_PLAYER_CARDS: {
                draw->card_rotation[signals[i].player_index][signals[i].card_index] = {
                    true,
                    true,
                    180.0f,
                    0.0f,
                    -draw->card_flipping_speed
                };
                //flip_card_model(draw, signals[i].player_index, signals[i].card_index);
                play_sound("TAP");
            } break;
            
            case SIGNAL_NEXT_PLAYER_ROTATION: {
                draw->camera_rotation = {
                    true,
                    true,
                    draw->degrees_between_players,
                    0.0f,
                    -draw->rotation_speed
                };
                play_sound("WOOSH");

            } break;

            case SIGNAL_NEW_CARD_ROTATION: {
                draw->new_card_rotation = {
                    true,
                    true,
                    180.0f,
                    0.0f,
                    -draw->card_flipping_speed
                };
            } break;

            case SIGNAL_NAME_PLATES: load_name_plates(game, draw); break;

            case SIGNAL_UNLOAD_NAME_PLATE: unload_name_plate(draw, signals[i].player_index); break;
        }
        signals[i].in_use = false;    
    }
}
