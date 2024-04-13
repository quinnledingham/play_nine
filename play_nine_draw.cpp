//
// Model
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
                render_bind_pipeline(&color_pipeline); 
                render_bind_descriptor_set(color_set);
            break;

            case 1:
                object.index = front_index;
            case 2: 
                render_bind_pipeline(&basic_pipeline); 
                render_bind_descriptor_set(texture_desc);
            break;
        }

        render_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
        render_draw_mesh(&model->meshes[draw_index]);
    }
}

// drawing highlight
void draw_highlight(Model *model, Render_Pipeline *color_pipeline, Vector4 color, Matrix_4x4 model_matrix) {
    render_bind_pipeline(color_pipeline);

    Descriptor color_set = render_get_descriptor_set(&layouts[5]);
    render_update_ubo(color_set, (void *)&color);
    render_bind_descriptor_set(color_set);

    render_push_constants(SHADER_STAGE_VERTEX, (void *)&model_matrix, sizeof(Matrix_4x4));

    for (u32 i = 0; i < model->meshes_count; i++) {
        render_draw_mesh(&model->meshes[i]);
    }
}

//
// Drawing
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

internal Matrix_4x4
load_card_model(bool8 flipped, Vector3 position, float32 rads, Vector3 scale) {
    Quaternion rotation = get_rotation(rads, {0, 1, 0});
    if (!flipped) {
        Quaternion flip = get_rotation(180.0f * DEG2RAD, {0, 0, 1});
        rotation = flip * rotation;
        position.y += (0.101767f * scale.y); // Hardcoded card.obj height
        return create_transform_m4x4(position, rotation, scale);
    } else
        return create_transform_m4x4(position, rotation, scale);
}

internal float32
get_pile_y_scale(u32 cards) {
    float32 max_y_scale = 10.0f;
    float32 min_y_scale = 0.5f;
    float32 percent = float32(cards) / float32(DECK_SIZE);
    return (max_y_scale * percent) + min_y_scale;
}

internal void
load_card_models(Game *game, Game_Draw *draw, float32 rotation_degrees) {
    Vector3 card_scale           = {1.0f, 0.5f, 1.0f};
    Vector3 selected_card_coords = {0.0f, 1.0f, -2.7f};
    Vector3 pile_coords          = { -1.1f, 0.0f, -2.0f };
    Vector3 discard_pile_coords  = {  1.1f, 0.0f, 0 };

    Vector3 active_player_position = {};

    for (u32 i = 0; i < game->num_of_players; i++) {
        float32 degrees = (draw->degrees_between_players * i) - 90.0f;
        float32 rad = degrees * DEG2RAD; 
        Vector3 position = get_hand_position(draw->radius, draw->degrees_between_players, i);
        
        if (i == 0)
            active_player_position = position;

        // draw player cards    
        for (u32 card_index = 0; card_index < 8; card_index++) {
            Vector3 card_pos = { hand_coords[card_index].x, 0.0f, hand_coords[card_index].y };
            rotate_coords(&card_pos, rad);
            card_pos += position;
            draw->hand_models[i][card_index] = load_card_model(game->players[i].flipped[card_index], card_pos, rad, card_scale);
        }
    }

    // move piles closer to player

    float32 center_of_piles_z = -active_player_position.x + 5.7f;
    pile_coords.z = center_of_piles_z;
    discard_pile_coords.z = center_of_piles_z;
    selected_card_coords.z = center_of_piles_z + selected_card_coords.z;

    // draw card selected from pile
    float32 degrees = (draw->degrees_between_players * game->active_player) - 90.0f;
    float32 rad = degrees * DEG2RAD; 
    if (game->turn_stage == SELECT_CARD) {
        rotate_coords(&selected_card_coords, rad);
        draw->new_card_model = load_card_model(true, selected_card_coords, rad, card_scale);
    }

    float32 rotation_rads = (degrees - rotation_degrees) * DEG2RAD;

    //Vector3 dir = normalized(active_player_position);
    
    rotate_coords(&pile_coords, rotation_rads);
    rotate_coords(&discard_pile_coords, rotation_rads);

    // pile
    {
        float32 pile_y_scale = get_pile_y_scale(DECK_SIZE - game->top_of_pile);
        draw->top_of_pile_model = load_card_model(false, pile_coords, rotation_rads, {1.0f, pile_y_scale, 1.0f});
    }

    // discard pile
    if (game->top_of_discard_pile != 0) {
        float32 pile_y_scale = get_pile_y_scale(game->top_of_discard_pile);
        draw->top_of_discard_pile_model = load_card_model(true, discard_pile_coords, rotation_rads, {1.0f, pile_y_scale, 1.0f});
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
        draw_highlight(card_model, &color_pipeline, highlight_color, model_scale);
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

    for (u32 i = 0; i < game->num_of_players; i++) {
        for (u32 card_index = 0; card_index < HAND_SIZE; card_index++) {
            s32 card = deck[game->players[i].cards[card_index]];
            bool8 h = highlight && i == game->active_player;
            switch (stage) {
                case SELECT_PILE: h = false; break;
                case FLIP_CARD: h = h && !game->players[i].flipped[card_index]; break;
            }

            draw_card(card_model, color_set, indices, card, draw->hand_models[i][card_index], h, get_highlight_color(draw, card_index), game->players[i].flipped[card_index]);
        }
    }
    
    {
        bool8 h = highlight && stage == SELECT_PILE;
        draw_card(card_model, color_set, indices, deck[game->pile[game->top_of_pile]], draw->top_of_pile_model, h, get_highlight_color(draw, PICKUP_PILE), false);
    }

    if (game->top_of_discard_pile != 0) {
        bool8 h = highlight && (stage == SELECT_PILE || stage == SELECT_CARD);
        if (stage == SELECT_CARD && !game->pile_card)
            h = false;
        draw_card(card_model, color_set, indices, deck[game->discard_pile[game->top_of_discard_pile - 1]], draw->top_of_discard_pile_model, h, get_highlight_color(draw, DISCARD_PILE), true);
    }

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

        draw->name_plates[i] = create_string_into_bitmap(default_font, 350.0f, game->players[i].name);
        render_create_texture(&draw->name_plates[i], TEXTURE_PARAMETERS_CHAR);
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

//      180
// 270        90
//       0
internal void
draw_game(State *state, Assets *assets, Shader *shader, Game *game, s32 indices[16]) {
    render_depth_test(true);

    render_bind_pipeline(&basic_pipeline);
    render_bind_descriptor_set(state->scene_set);
    render_bind_descriptor_set(light_set);

    // Skybox
    draw_cube({ 0, 0, 0 }, 0.0f, { 100, 100, 100 }, { 30, 20, 10, 1 });

    // Table
    render_bind_pipeline(&basic_pipeline);
    render_bind_descriptor_set(texture_desc);

    Object object = {};
    object.model = create_transform_m4x4({ 0, -0.1f, 0 }, get_rotation(0, { 0, 1, 0 }), {15.6f, 1.0f, 15.6f});
    object.index = indices[15];
    render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

    Model *model = find_model(assets, "TABLE");
    for (u32 i = 0; i < model->meshes_count; i++) { 
        render_draw_mesh(&model->meshes[i]);
    }

    render_bind_pipeline(&text_pipeline);
    render_bind_descriptor_set(light_set);

    // Name Plate
    //win32_wait_mutex(state->mutex);
    draw_name_plates(game, &state->game_draw);
    //win32_release_mutex(state->mutex);

    // Cards
    render_bind_descriptor_set(texture_desc);

    Model *card_model = find_model(assets, "CARD");
    draw_cards(game, &state->game_draw, card_model, false, indices);

    render_depth_test(false);

    bool8 online_bool = state->client_game_index == state->game.active_player || (!state->is_client && !state->is_server);
    if (state->game.round_type != HOLE_OVER && online_bool && !state->game_draw.camera_rotation.rotating)
        draw_cards(game, &state->game_draw, card_model, true, indices);
}