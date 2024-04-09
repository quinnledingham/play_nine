#include <stdarg.h>
#include <cstdint>
#include <ctype.h>
#include <stdio.h>

#include "defines.h"
#include "types.h"
#include "types_math.h"

void *platform_malloc(u32 size);
void platform_free(void *ptr);
void platform_memory_set(void *dest, s32 value, u32 num_of_bytes);

#include "print.h"
#include "char_array.h"
#include "assets.h"

#include "application.h"
#include "play_nine_input.h"
#include "play_nine.h"

#include "raytrace.cpp"

internal void
default_player_name_string(char buffer[MAX_NAME_SIZE], u32 number) {
    platform_memory_set((void*)buffer, 0, MAX_NAME_SIZE);
    buffer[8] = 0;
    memcpy(buffer, "Player ", 7);
    buffer[7] = number + 48; // + 48 turns single digit number into ascii value
}
/*
n_o_p = 5
index = 2

0 1 2 3 4

0 1 3 4

0 1 2 3
*/

internal void
remove_player(Game *game, u32 index) {
    u32 dest_index = index;
    u32 src_index = index + 1;
    for (src_index; src_index < game->num_of_players; src_index++) {
        game->players[dest_index++] = game->players[src_index];
    }
    game->num_of_players--;
}

#include "play_nine_online.cpp"

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
    u8 *dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch);
    u8 *src_ptr = src.memory;
    for (s32 y = 0; y < src.height; y++) {
        for (s32 x = 0; x < src.width; x++) { 
            if (dest.channels == 1) {
                *dest_ptr = *src_ptr;
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

    float32 current_point = 0.0f;

    s32 height = 0;
    float32 left = 0.0f;

    u32 i = 0;
    while (str[i] != 0 ) {
        Font_Char_Bitmap *fbitmap = load_font_char_bitmap(font, str[i], scale);
        Font_Char *font_char = fbitmap->font_char;

        Vector2 char_coords = { current_point + (font_char->lsb * scale), (float32)fbitmap->bb_0.y };
        s32 char_height = fbitmap->bb_1.y - fbitmap->bb_0.y;

        if (char_height > height)
            height = char_height;
        if (char_coords.x < 0)
            left = char_coords.x;

        s32 kern = get_codepoint_kern_advance(font->info, str[i], str[i + 1]);
        current_point += scale * (kern + font_char->ax);
        
        i++;
    }

    bitmap.width = s32(current_point - left);
    bitmap.height = s32(height);
    bitmap.channels = 1;
    bitmap.pitch = bitmap.width * bitmap.channels;
    bitmap.memory = (u8*)platform_malloc(bitmap.width * bitmap.height * bitmap.channels);
    memset(bitmap.memory, 0x00, bitmap.width * bitmap.height * bitmap.channels);

    current_point = 0.0f;

    i = 0;
    while (str[i] != 0 ) {
        Font_Char_Bitmap *fbitmap = load_font_char_bitmap(font, str[i], scale);
        Font_Char *font_char = fbitmap->font_char;

        s32 char_height = fbitmap->bb_1.y - fbitmap->bb_0.y;
        Vector2 char_coords = { current_point + (font_char->lsb * scale) - left, ((float32)height / 2.0f) - ((float32)char_height / 2.0f)};

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
    for (s32 i = 1; i <= number; i++) {
        float32 y = (bitmap.height * (float32(i) / float32(number + 1))) - radius;
        copy_blend_bitmap(bitmap, circle_bitmap, { x, s32(y) }, color);
    }
}

internal void
add_balls_to_bitmap(Bitmap bitmap, Bitmap circle_bitmap, s32 number) {
    float32 x = 0.0f;
    float32 y = 0.0f;

    float32 radius = (circle_bitmap.width  / 2.0f);

    s32 *design = rows[number];
    s32 columns = 0; // count how many columns are in this design to only space for that many
    for (u32 i = 0; i < 3; i++) {
        if (design[i] > 0)
            columns++;
    }

    // draw each column that is not zero
    s32 columns_index = 1; 
    for (u32 i = 0; i < 3; i++) {
        if (design[i] > 0) {
            x = (bitmap.width * (columns_index++ / float32(columns + 1))) - radius;
            add_balls_line(bitmap, circle_bitmap, s32(x), design[i], ball_colors[number]);
        }
    }
    
}

internal Bitmap
create_card_bitmap(Font *font, s32 number, Bitmap circle_bitmap) {
    Bitmap bitmap   = {};
    bitmap.width    = 1024;
    bitmap.height   = 1638;
    bitmap.channels = 4;
    bitmap.pitch    = bitmap.width * bitmap.channels;
    bitmap.memory   = (u8*)platform_malloc(bitmap.width * bitmap.height * bitmap.channels);
    memset(bitmap.memory, 0xFF, bitmap.width * bitmap.height * bitmap.channels);

    add_balls_to_bitmap(bitmap, circle_bitmap, number);

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

    Bitmap str_bitmap = create_string_into_bitmap(font, 350.0f, str);

    s32 padding = 50;
    Vector3 text_color = { 0, 0, 0 };
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
    Bitmap circle_bitmap = create_circle_bitmap({ 200, 200 });

    for (s32 i = 0; i <= 13; i++) {
        bitmaps[i] = create_card_bitmap(font, i, circle_bitmap);
    }

    platform_free(circle_bitmap.memory);
}

internal void
draw_card_bitmaps(Bitmap bitmaps[14], Vector2_s32 window_dim) {
    Vector2 pos = { 0, 0 };
    Vector2 dim = { window_dim.x / 14.0f, 0 };
    float32 percent = dim.x / bitmaps[0].width;
    dim.y = bitmaps[0].height * percent;
    for (u32 i = 0; i < 14; i++) {
        pos.x = i * dim.x;
        draw_rect(pos, 0, dim, &bitmaps[i]);
    }
}

//
// drawing card
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

            if (rot->degrees > rot->dest_degrees)
                rot->degrees = rot->dest_degrees;
        } else {
            rot->rotating = false;
        }
    } else if (rot->speed < 0.0f) { 
        if (rot->degrees > rot->dest_degrees) {
            rot->degrees += (seconds * rot->speed);

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
    Vector3 pile_coords          = { -1.1f, 0.0f, 0 };
    Vector3 discard_pile_coords  = {  1.1f, 0.0f, 0 };

    for (u32 i = 0; i < game->num_of_players; i++) {
        float32 degrees = (draw->degrees_between_players * i) - 90.0f;
        float32 rad = degrees * DEG2RAD; 
        Vector3 position = get_hand_position(draw->radius, draw->degrees_between_players, i);
        
        // draw player cards    
        for (u32 card_index = 0; card_index < 8; card_index++) {
            Vector3 card_pos = { hand_coords[card_index].x, 0.0f, hand_coords[card_index].y };
            rotate_coords(&card_pos, rad);
            card_pos += position;
            game->players[i].models[card_index] = load_card_model(game->players[i].flipped[card_index], card_pos, rad, card_scale);
        }
    }

    // draw card selected from pile
    float32 degrees = (draw->degrees_between_players * game->active_player) - 90.0f;
    float32 rad = degrees * DEG2RAD; 
    if (game->players[game->active_player].turn_stage == SELECT_CARD) {
        rotate_coords(&selected_card_coords, rad);
        game->players[game->active_player].new_card_model = load_card_model(true, selected_card_coords, rad, card_scale);
    }

    float32 rotation_rads = (degrees - rotation_degrees) * DEG2RAD;

    rotate_coords(&pile_coords, rotation_rads);
    rotate_coords(&discard_pile_coords, rotation_rads);

    // pile
    {
        float32 pile_y_scale = get_pile_y_scale(DECK_SIZE - game->top_of_pile);
        game->top_of_pile_model = load_card_model(false, pile_coords, rotation_rads, {1.0f, pile_y_scale, 1.0f});
    }

    // discard pile
    if (game->top_of_discard_pile != 0) {
        float32 pile_y_scale = get_pile_y_scale(game->top_of_discard_pile);
        game->top_of_discard_pile_model = load_card_model(true, discard_pile_coords, rotation_rads, {1.0f, pile_y_scale, 1.0f});
    }
}

internal void
draw_card(Model *card_model, Descriptor color_set, s32 indices[16], u32 number, Matrix_4x4 model, bool8 highlight, Vector4 highlight_color, bool8 flipped) {
    u32 bitmap_index = number;
    if (number == -5)
        bitmap_index = 13;

    if (highlight) {
        Matrix_4x4 model_scale = m4x4_scale(model, { 1.06f, 1.06f, 1.06f });
        render_draw_model(card_model, &color_pipeline, highlight_color, model_scale);
    }

    render_draw_model(card_model, color_set, indices[bitmap_index], indices[14], model, flipped);
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

//      180
// 270        90
//       0
internal void
draw_game(State *state, Assets *assets, Shader *shader, Game *game, bool8 highlight, s32 indices[16]) {
    Descriptor color_set = render_get_descriptor_set(&layouts[5]);

    Vector4 color = { 150, 150, 150, 1 };
    render_update_ubo(color_set, &color);

    Model *card_model = find_model(assets, "CARD");

    if (!highlight) {
        draw_cube({ 0, 0, 0 }, 0.0f, { 100, 100, 100 }, { 30, 20, 10, 1 });
        render_bind_pipeline(&basic_pipeline);
        render_bind_descriptor_set(texture_desc);

        Object object = {};
        object.model = create_transform_m4x4({ 0, -0.1f, 0 }, get_rotation(0, { 0, 1, 0 }), {15.0f, 1.0f, 15.0f});
        object.index = indices[15];
        render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

        Model *model = find_model(assets, "TABLE");
        for (u32 i = 0; i < model->meshes_count; i++) { 
            render_draw_mesh(&model->meshes[i]);
        }
    }

    Player *active_player = &game->players[game->active_player];
    enum Turn_Stages stage = active_player->turn_stage;

    for (u32 i = 0; i < game->num_of_players; i++) {
        for (u32 card_index = 0; card_index < HAND_SIZE; card_index++) {
            s32 card = deck[game->players[i].cards[card_index]];
            bool8 h = highlight && i == game->active_player;
            switch (stage) {
                case SELECT_PILE: h = false; break;
                case FLIP_CARD: h = h && !game->players[i].flipped[card_index]; break;
            }

            draw_card(card_model, color_set, indices, card, game->players[i].models[card_index], h, get_highlight_color(&state->game_draw, card_index), game->players[i].flipped[card_index]);
        }
    }
    
    {
        bool8 h = highlight && stage == SELECT_PILE;
        draw_card(card_model, color_set, indices, deck[game->pile[game->top_of_pile]], game->top_of_pile_model, h, get_highlight_color(&state->game_draw, PICKUP_PILE), false);
    }

    if (game->top_of_discard_pile != 0) {
        bool8 h = highlight && (stage == SELECT_PILE || stage == SELECT_CARD);
        if (stage == SELECT_CARD && !game->players[game->active_player].pile_card)
            h = false;
        draw_card(card_model, color_set, indices, deck[game->discard_pile[game->top_of_discard_pile - 1]], game->top_of_discard_pile_model, h, get_highlight_color(&state->game_draw, DISCARD_PILE), true);
    }

    if (stage == SELECT_CARD) {
        s32 card = deck[game->players[game->active_player].new_card];
        draw_card(card_model, color_set, indices, card, game->players[game->active_player].new_card_model, false, play_nine_yellow, true);
    }
}

internal void
init_deck() {
    u32 deck_index = 0;
    for (s32 i = 0; i <= 12; i++) {
        for (u32 j = deck_index; j < deck_index + 8; j++) {
            deck[j] = i;
        }
        deck_index += 8;
    }

    for (u32 j = deck_index; j < deck_index + 4; j++) {
        deck[j] = -5;
    }

    // init hand coords
    float32 card_width = 2.0f;
    float32 card_height = 3.2f;
    float32 padding = 0.2f;
    hand_width = card_width * 4.0f + padding * 3.0f;

    u32 card_index = 0;
    for (s32 y = 1; y >= 0; y--) {
        for (s32 x = -2; x <= 1; x++) {
            hand_coords[card_index++] = { 
                float32(x) * card_width  + (float32(x) * padding) + (card_width / 2.0f)  + (padding / 2.0f), 
                float32(y) * card_height + (float32(y) * padding) - (card_height / 2.0f) - (padding / 2.0f)
            };
        }
    }
}

// srand at beginning of main_loop()
internal s32
random(s32 lower, s32 upper) {
    return lower + (rand() % (upper - lower));
}

internal void
shuffle_pile(u32 *pile) {
    for (u32 i = 0; i < DECK_SIZE; i++) {
        bool8 not_new_card = true;
        while(not_new_card) {
            pile[i] = random(0, DECK_SIZE);
            not_new_card = false;
            for (u32 j = 0; j < i; j++) {
                if (pile[i] == pile[j])
                    not_new_card = true;
            }
        }
    }
}

internal void
deal_cards(Game *game) {
     for (u32 i = 0; i < game->num_of_players; i++) {
         for (u32 card_index = 0; card_index < HAND_SIZE; card_index++) {
            game->players[i].cards[card_index] = game->pile[game->top_of_pile++];
            game->players[i].flipped[card_index] = false;
         }

        game->players[i].turn_stage = FLIP_CARD;
     }

    game->discard_pile[game->top_of_discard_pile++] = game->pile[game->top_of_pile++];
}

internal void
start_hole(Game *game) {
    game->top_of_pile = 0;
    game->top_of_discard_pile = 0;
    shuffle_pile(game->pile);
    deal_cards(game);
    game->active_player = 0;
    game->round_type = FLIP_ROUND;
    game->last_turn = 0;
}

internal void
start_game(Game *game, u32 num_of_players) {
    game->holes_played = 0;
    game->num_of_players = num_of_players;
    game->game_over = false;
    start_hole(game);
}

internal void
reset_game(Game *game) {
    //game->num_of_players = 1;
    //for (u32 i = 0; i < 6; i++)
    //    platform_memory_set(game->players[i].cards, 0, sizeof(u32) * HAND_SIZE);
    //game->active_player = 0;
}

internal float32
get_draw_radius(u32 num_of_players, float32 hand_width, float32 card_height) {
    if (num_of_players == 2) {
        return 2.0f * card_height + 0.2f;
    }

    float32 angle = (360.0f / float32(num_of_players)) / 2.0f;
    float32 radius = (hand_width / 2.0f) / tanf(angle * DEG2RAD);
    return radius + card_height + 0.1f;
}

internal Game
get_test_game() {
    Game game = {};
    game.num_of_players = 2;
    game.holes_played = 2;

    for (u32 i = 0; i < game.num_of_players; i++) {
        default_player_name_string(game.players[i].name, i);
        game.players[i].scores[0] = i;
    }

    game.players[0].scores[0] = -10;

    return game;
}

#include "play_nine_menus.cpp"

//
// Scoring
// 

struct Card_Pair {
    bool8 matching;
    s32 score;
    s32 number;
};

inline Card_Pair
create_pair(u32 top, u32 bottom) {
    Card_Pair result = {};
    s32 top_card = deck[top];
    s32 bottom_card = deck[bottom];

    if (top_card == bottom_card) {
        result.matching = true;
        result.number = top_card; // matching so top and bottom same number
    }

    if (!result.matching || (top_card == -5))
        result.score += top_card + bottom_card;

    return result;
} 

struct Card_Pair_Match {
    s32 number;
    s32 pairs;
};

internal s32
get_score(u32 *cards) {
    s32 score = 0;

    Card_Pair pairs[4] = {
        create_pair(cards[0], cards[4]),
        create_pair(cards[1], cards[5]),
        create_pair(cards[2], cards[6]),
        create_pair(cards[3], cards[7])
    };

    for (u32 i = 0; i < 4; i++) {
        score += pairs[i].score;
    }

    u32 match_index = 0;
    Card_Pair_Match matches[4];
    for (u32 i = 0; i < 4; i++) {
        if (pairs[i].matching) {
            bool8 found_match = false;
            for (u32 j = 0; j < match_index; j++) {
                if (matches[j].number == pairs[i].number) {
                    matches[j].pairs++;
                    found_match = true;
                }
            }
            if (found_match)
                continue;
            matches[match_index++] = { pairs[i].number, 1 };
        }
    }

    for (u32 i = 0; i < match_index; i++) {
        Card_Pair_Match match = matches[i];
        switch(match.pairs) {
            case 2: score -= 10; break;
            case 3: score -= 15; break;
            case 4: score -= 20; break;
        }
    }

    return score;
}

internal void
update_scores(Game *game) {
    for (u32 i = 0; i < game->num_of_players; i++) {
        Player *player = &game->players[i];
        u32 *cards = player->cards;            
        s32 score = get_score(cards);
        //print("Player %d: %d\n", i, score);
        player->scores[game->holes_played] = score;
    }
}

//
// game logic
//

internal void
next_player(Game *game, Game_Draw *draw) {
    // Flip all cards after final turn
    if (game->round_type == FINAL_ROUND) {
        game->last_turn++;
        for (u32 i = 0; i < HAND_SIZE; i++) {
            game->players[game->active_player].flipped[i] = true;
        }
    }

    game->players[game->active_player].turn_stage = SELECT_PILE;
    game->players[game->active_player].pile_card = false;

    game->active_player++;

    // END HOLE
    if (game->last_turn == game->num_of_players && game->round_type != HOLE_OVER) {
        update_scores(game);
        game->holes_played++;
        game->round_type = HOLE_OVER;

        // END GAME
        if (game->holes_played == GAME_LENGTH) {
            game->game_over = true;
        }
    }

    // end of round: loop back around if required
    if (game->active_player >= game->num_of_players) {
        game->active_player = 0;

        if (game->round_type == FLIP_ROUND)
            game->round_type = REGULAR_ROUND;
    }

    // Update for draw
    //add_onscreen_notification(&draw->notifications, game->players[game->active_player].name);

    draw->camera_rotation = {
        true,
        draw->degrees_between_players,
        0.0f,
        -draw->rotation_speed
    };

    draw->pile_rotation = {
        true,
        draw->degrees_between_players,
        0.0f,
        -draw->rotation_speed 
    };
}

internal u32
get_number_flipped(bool8 *flipped) {
    u32 number_flipped = 0;
    for (u32 i = 0; i < HAND_SIZE; i++) {
        if (flipped[i]) {
            number_flipped++;
        }
    }
    return number_flipped;
}

internal void
flip_round_update(Game *game, Game_Draw *draw, bool8 selected[SELECTED_SIZE]) {
    Player *active_player = &game->players[game->active_player];
    for (u32 i = 0; i < HAND_SIZE; i++) {
        if (selected[i]) {
            active_player->flipped[i] = true;

            // check if player turn over
            if (get_number_flipped(active_player->flipped) == 2) {
                next_player(game, draw);
                return;
            }
        }
    }
}

internal void
regular_round_update(Game *game, Game_Draw *draw, bool8 selected[SELECTED_SIZE]) {
    Player *active_player = &game->players[game->active_player];
    switch(active_player->turn_stage) {
        case SELECT_PILE: {
            if (selected[PICKUP_PILE]) {
                active_player->new_card = game->pile[game->top_of_pile++];
    
                // Probably shouldn't happen in a real game
                if (game->top_of_pile + game->num_of_players >= DECK_SIZE) {
                    game->round_type = FINAL_ROUND;
                }

                active_player->turn_stage = SELECT_CARD;
                active_player->pile_card = true;
            } else if (selected[DISCARD_PILE]) {
                active_player->new_card = game->discard_pile[game->top_of_discard_pile - 1];
                game->top_of_discard_pile--;
                active_player->turn_stage = SELECT_CARD;
            }
        } break;

        case SELECT_CARD: {
            for (u32 i = 0; i < HAND_SIZE; i++) {
                if (selected[i]) {
                    game->discard_pile[game->top_of_discard_pile++] = active_player->cards[i];

                    active_player->flipped[i] = true;
                    active_player->cards[i] = active_player->new_card;
                    active_player->new_card = 0;

                    if (get_number_flipped(active_player->flipped) == HAND_SIZE)
                        game->round_type = FINAL_ROUND;
                    next_player(game, draw);
                    return;
                }
            }

            if (active_player->pile_card && selected[DISCARD_PILE]) {
                game->discard_pile[game->top_of_discard_pile++] = active_player->new_card;
                active_player->new_card = 0;
                active_player->turn_stage = FLIP_CARD;
            }
        } break;

        case FLIP_CARD: {
            for (u32 i = 0; i < HAND_SIZE; i++) {
                if (selected[i] && !active_player->flipped[i]) {
                    active_player->flipped[i] = true;
                    if (get_number_flipped(active_player->flipped) == HAND_SIZE) {
                        game->round_type = FINAL_ROUND; 
                        game->last_turn = 0;
                    }
                    next_player(game, draw);
                    return;
                }
/*
                if (selected[DISCARD_PILE] && get_number_flipped(active_player->flipped) == HAND_SIZE - 1) {
                    next_player(game, draw);
                    return;
                }
*/
                if (selected[PASS_BUTTON] && get_number_flipped(active_player->flipped) == HAND_SIZE - 1) {
                    next_player(game, draw);
                    return;
                }

            }
        } break;
    }
}

internal bool8
ray_model_intersection8(Ray ray, Model *model, Matrix_4x4 card) {
    for (u32 i = 0; i < model->meshes_count; i++) {
        Ray_Intersection p = intersect_triangle_mesh(ray, &model->meshes[i], card);
        if (p.number_of_intersections != 0) {
            print("card: %f %f %f\n", p.point.x, p.point.y, p.point.z);
            return true;
        }
    }

    return false;
}

Descriptor ray_desc;
Descriptor tri_desc;
Descriptor out_desc;

internal bool8
ray_model_intersection(Ray ray, Model *model, Matrix_4x4 card) {
    vulkan_start_compute();
    vulkan_bind_pipeline(&ray_pipeline);

    //Descriptor ray_desc = vulkan_get_descriptor_set_index(&layouts[6], 0);
    //Descriptor tri_desc = render_get_descriptor_set_index(&layouts[7], 0);
    //Descriptor out_desc = vulkan_get_descriptor_set_index(&layouts[8], 0);

    vulkan_bind_descriptor_set(ray_desc);
    vulkan_bind_descriptor_set(tri_desc);
    vulkan_bind_descriptor_set(out_desc);

    Object object = {};
    object.model = card;
    render_push_constants(SHADER_STAGE_COMPUTE, (void *)&object, sizeof(Object)); 

    vulkan_dispatch(1, 1, 1);
    vulkan_end_compute();

    Ray_Intersection p = *((Ray_Intersection*)vulkan_info.storage_buffer.data + out_desc.offset);

    if (p.number_of_intersections != 0) {
        print("card: %f %f %f\n", p.point.x, p.point.y, p.point.z);
        return true;
    }

    return false;
}

internal void
mouse_ray_model_intersections(bool8 *selected, Ray mouse_ray, Game *game, Model *card_model) {
    Player *active_player = &game->players[game->active_player];

    for (u32 card_index = 0; card_index < 8; card_index++) {
        selected[card_index] = ray_model_intersection(mouse_ray, card_model, active_player->models[card_index]);
        if (selected[card_index]) return;
    }

    selected[PICKUP_PILE] = ray_model_intersection(mouse_ray, card_model, game->top_of_pile_model);
    if (game->top_of_discard_pile != 0)
        selected[DISCARD_PILE] = ray_model_intersection(mouse_ray, card_model, game->top_of_discard_pile_model);
}

internal void
do_mouse_selected_update(State *state, App *app, bool8 selected[SELECTED_SIZE]) {
    Game *game = &state->game;
    Game_Draw *draw = &state->game_draw;
    Model *card_model = find_model(&state->assets, "CARD");

    set_ray_coords(&state->mouse_ray, state->camera, state->scene.projection, state->scene.view, app->input.mouse, app->window.dim);\

    vulkan_start_compute();
    vulkan_bind_pipeline(&ray_pipeline);

    ray_desc = vulkan_get_descriptor_set_index(&layouts[6], 0);
    tri_desc = render_get_descriptor_set_index(&layouts[7], 0);
    out_desc = vulkan_get_descriptor_set_index(&layouts[8], 0);

    vulkan_update_ubo(ray_desc, &state->mouse_ray);
    vulkan_set_storage_buffer(out_desc);

    //vulkan_bind_descriptor_set(ray_desc);
    //vulkan_bind_descriptor_set(tri_desc);
    //vulkan_bind_descriptor_set(out_desc);

    //vulkan_dispatch(1, 1, 1);

    vulkan_end_compute();

    //Ray_Intersection test = *((Ray_Intersection*)vulkan_info.storage_buffer.data + out_desc.offset);
    //print("%d\n", test.number_of_intersections);
    
    mouse_ray_model_intersections(draw->highlight_hover, state->mouse_ray, game, card_model);

    if (on_down(state->controller.mouse_left)) {
        platform_memory_copy(draw->highlight_pressed, draw->highlight_hover, sizeof(bool8) * SELECTED_SIZE);
    }

    if (on_up(state->controller.mouse_left)) {
        platform_memory_copy(selected, draw->highlight_hover, sizeof(bool8) * SELECTED_SIZE);

        for (u32 i = 0; i < SELECTED_SIZE; i++) {
            if (selected[i] && !draw->highlight_pressed[i])
                selected[i] = false;
        }

        platform_memory_set(draw->highlight_pressed, 0, sizeof(bool8) * SELECTED_SIZE);
    }
}

internal void
do_controller_selected_update(bool8 selected[SELECTED_SIZE], Game *game, Controller *controller) {
    for (u32 i = 0; i < HAND_SIZE; i++) {
        if (on_up(controller->buttons[i])) {
            selected[i] = true;
        }
    }    

    if (on_up(controller->nine)) {
        selected[PICKUP_PILE] = true;
    } else if (on_up(controller->zero)) {
        selected[DISCARD_PILE] = true;
    }
}

internal bool8
all_false(bool8 selected[SELECTED_SIZE]) {
    for (u32 i = 0; i < SELECTED_SIZE; i++) {
        if (selected[i])
            return false;
    }
    return true;
}

bool8 update_game(State *state, App *app) {
    Game *game = &state->game;
    Game_Draw *draw = &state->game_draw;

    // Change camera mode or menu mode
    switch(state->camera_mode) {
        case FREE_CAMERA: {
            if (on_down(state->controller.pause)) {
                state->menu_list.mode = PAUSE_MENU;
                app->input.relative_mouse_mode = false;
            }
    
            if (on_down(state->controller.camera_toggle)) {
                state->camera_mode = PLAYER_CAMERA;
                app->input.relative_mouse_mode = false;
            }
        } break;

        case PLAYER_CAMERA: {
            if (on_down(state->controller.pause)) {
                state->menu_list.mode = PAUSE_MENU;
            }
            
            if (on_down(state->controller.camera_toggle)) {
                state->camera_mode = FREE_CAMERA;
                app->input.relative_mouse_mode = true;

                state->camera.position = Vector3{ 0, 0, 1 };
                state->camera.yaw = 270.0f;
                state->camera.pitch = 0.0f;
                break;
            }
        } break;
    }
    
    // Game update
    switch(state->camera_mode) {
        case PLAYER_CAMERA: {
            // Game input                      
            if (on_down(state->controller.right)) {
                next_player(game, &state->game_draw);
            }

            bool8 selected[SELECTED_SIZE] = {};
            if (state->client_game_index == game->active_player || (!state->is_client && !state->is_server)) {
                do_mouse_selected_update(state, app, selected);
                do_controller_selected_update(selected, game, &state->controller);

                if (state->pass_selected) {
                    selected[PASS_BUTTON] = true;
                    state->pass_selected = false;
                }

                if (state->is_client && !all_false(selected)) {
                    client_set_selected(state->client, selected, state->client_game_index);
                }
            } else if (state->is_server) {
                win32_wait_mutex(state->selected_mutex);
                if (!all_false(state->selected)) {
                    platform_memory_copy(selected, state->selected, sizeof(selected[0]) * SELECTED_SIZE);
                    platform_memory_set(state->selected, 0, sizeof(selected[0]) * SELECTED_SIZE);
                }
                win32_release_mutex(state->selected_mutex);
            }

            // Game logic
            switch(game->round_type) {
                case FLIP_ROUND:    flip_round_update(game, &state->game_draw, selected); break;
                case FINAL_ROUND:
                case REGULAR_ROUND: regular_round_update(game, &state->game_draw, selected); break;
            }
        } break;
    }

    // Update camera and card models after what happened in game update
    float32 rotation_degrees = process_rotation(&draw->camera_rotation, (float32)app->time.frame_time_s);

    switch(state->camera_mode) {
        case FREE_CAMERA: {
            if (app->input.relative_mouse_mode) {
                float64 mouse_m_per_s = 50.0;
                //float64 mouse_move_speed = mouse_m_per_s * app->time.frame_time_s;
                float64 mouse_move_speed = 0.1f;
                //print("%d %d == %d %d\n", mouse.x, mouse.y, app->input.mouse_rel.x, app->input.mouse_rel.y);
                update_camera_with_mouse(&state->camera, app->input.mouse_rel, mouse_move_speed, mouse_move_speed);
                update_camera_target(&state->camera);    
    
                // I want target to be updated before I do this because it uses it
                float32 m_per_s = 6.0f; 
                float32 m_moved = m_per_s * (float32)app->time.frame_time_s;
                Vector3 move_vector = {m_moved, m_moved, m_moved};
                update_camera_with_keys(&state->camera, state->camera.target, state->camera.up, move_vector,
                                        is_down(state->controller.forward),  is_down(state->controller.backward),
                                        is_down(state->controller.left),  is_down(state->controller.right),
                                        is_down(state->controller.up),  is_down(state->controller.down));                             

                //print("%f %f %f\n", state->camera.position.x, state->camera.position.y, state->camera.position.z);
            }
        } break;

        case PLAYER_CAMERA: {
            // update camera after game logic

            float32 cam_dis = 8.0f + draw->radius;
            float32 deg = -draw->degrees_between_players * game->active_player;
            deg += rotation_degrees;
            float32 rad = deg * DEG2RAD;
            float32 x = cam_dis * cosf(rad);
            float32 y = cam_dis * sinf(rad);

            state->camera.position =  Vector3{ x, 14.0f, y };
            state->camera.yaw      = deg + 180.0f;
            state->camera.pitch    = -51.0f;

            update_camera_target(&state->camera);    
        } break;
    }

    state->scene.view = get_view(state->camera);
    render_update_ubo(state->scene_set, (void *)&state->scene);

    load_card_models(game, draw, rotation_degrees);

    return 0;
}

/*
https://www.freepik.com/free-vector/simple-realistic-wood-texture_1008177.htm#query=cartoon%20wood%20texture&position=3&from_view=keyword&track=ais&uuid=3c2d0918-a699-4f9b-b835-791d1dd2e14f
*/

bool8 init_data(App *app) {
	app->data = platform_malloc(sizeof(State));
    State *game = (State *)app->data;
    *game = {};
	game->assets = {};

    bool8 load_and_save_assets = true;

    if (load_and_save_assets) {
        if (load_assets(&game->assets, "../assets.ethan"))
            return true;
    } else {
        const char *filepath = "assets.save";
        u32 offset = 0;

        // have to compile then run the asset builder application to put the assets in the exe
        // const char *filepath = "river.exe";
        // u32 offset = exe_offset;

        if (load_saved_assets(&game->assets, filepath, offset))
            return 1;
    }

    init_assets(&game->assets);

    default_font = find_font(&game->assets, "CASLON");

    print("Bitmap Size: %d\nFont Size: %d\nShader Size: %d\nAudio Size: %d\nModel Size: %d\n", sizeof(Bitmap), sizeof(Font), sizeof(Shader), sizeof(Audio), sizeof(Model));

    if (load_and_save_assets) {
        init_card_bitmaps(card_bitmaps, default_font); 
        Asset *card_assets = ARRAY_MALLOC(Asset, 14);
        for (u32 i = 0; i < 14; i++) {
            Asset *asset = &card_assets[i];
            asset->type = ASSET_TYPE_BITMAP;
            asset->bitmap = card_bitmaps[i];
            asset->tag = (const char *)platform_malloc(5);
            asset->tag_length = 4;
            platform_memory_set((void*)asset->tag, 0, 5);
            const char *tag = "carb";
            platform_memory_copy((void*)asset->tag, (void*)tag, 4);
        };
        add_assets(&game->assets, card_assets, 14);
        //print_assets(&game->assets);

        FILE *file = fopen("assets.save", "wb");
        save_assets(&game->assets, file);
        fclose(file);
    } else {
        for (u32 i = 0; i < 14; i++) {
            card_bitmaps[i] = game->assets.types[0].data[i + 3].bitmap;
        }
    }

    default_font = find_font(&game->assets, "CASLON");

    //clear_font_bitmap_cache(default_font);
    
    global_assets = &game->assets;
    init_layouts(layouts, find_bitmap(&game->assets, "DAVID"));

    u32 test = sizeof(Layout_Set);

	Shader *basic_3D = find_shader(&game->assets, "BASIC3D");
    init_basic_vert_layout(&basic_3D->set, layouts);
    init_basic_frag_layout(basic_3D, layouts);
    basic_pipeline.shader = basic_3D;
    basic_pipeline.depth_test = true;
	render_create_graphics_pipeline(&basic_pipeline, get_vertex_xnu_info());
	
    Shader *color_3D = find_shader(&game->assets, "COLOR3D");
    init_basic_vert_layout(&color_3D->set, layouts);
    init_color3D_frag_layout(color_3D, layouts);
    color_pipeline.shader = color_3D;
    color_pipeline.depth_test = true;
    render_create_graphics_pipeline(&color_pipeline, get_vertex_xnu_info());

    Shader *ray_comp = find_shader(&game->assets, "RAY");
    init_ray_comp_layout(&ray_comp->set, layouts);
    ray_pipeline.shader = ray_comp;
    vulkan_create_compute_pipeline(&ray_pipeline);

	// Rendering
    game->camera.position = { 0, 2, -5 };
    game->camera.target   = { 0, 0, 0 };
    game->camera.up       = { 0, -1, 0 };
    game->camera.fov      = 75.0f;
    game->camera.yaw      = 180.0f;
    game->camera.pitch    = -75.0f;
		
	set(&game->controller.forward, 'w');
    set(&game->controller.forward, SDLK_UP);
	set(&game->controller.backward, SDLK_s);
    set(&game->controller.backward, SDLK_DOWN);
	set(&game->controller.left, SDLK_a);
    set(&game->controller.left, SDLK_LEFT);
	set(&game->controller.right, SDLK_d);
    set(&game->controller.right, SDLK_RIGHT);

	set(&game->controller.up, SDLK_SPACE);
	set(&game->controller.down, SDLK_LSHIFT);

    set(&game->controller.select, SDLK_RETURN);
	set(&game->controller.pause,  SDLK_ESCAPE);
    set(&game->controller.pass,   SDLK_p); 

    set(&game->controller.camera_toggle, SDLK_c);

    set(&game->controller.one,   SDLK_y);
    set(&game->controller.two,   SDLK_u);
    set(&game->controller.three, SDLK_i);
    set(&game->controller.four,  SDLK_o);

    set(&game->controller.five,  SDLK_h);
    set(&game->controller.six,   SDLK_j);
    set(&game->controller.seven, SDLK_k);
    set(&game->controller.eight, SDLK_l);

    set(&game->controller.nine,  SDLK_9);
    set(&game->controller.zero,  SDLK_0);

    set(&game->controller.mouse_left, SDL_BUTTON_LEFT);

    init_deck();
    
    init_shapes(&game->assets);

    clean_assets(&game->assets);

    game->game_draw.rotation_speed = 150.0f;
    game->camera_mode = PLAYER_CAMERA;

    game->notifications.font = default_font;
    game->notifications.text_color = { 255, 255, 255, 1 };

    // Setting default Menus
    Menu default_menu = {};
    default_menu.font = default_font;

    default_menu.style.default_back = play_nine_yellow;
    default_menu.style.default_text = play_nine_green;
    default_menu.style.hover_back   = play_nine_light_yellow;
    default_menu.style.hover_text   = play_nine_green;
    //default_menu.style.hover_text = { 255, 255, 255, 1 };
    default_menu.style.pressed_back = play_nine_dark_yellow;
    default_menu.style.pressed_text = play_nine_green;
    default_menu.style.active_back  = play_nine_yellow;
    default_menu.style.active_text  = play_nine_green;
    //default_menu.style.active_text = { 255, 255, 255, 1 };

    default_menu.active_section = { -1, -1 };
    default_menu.edit.section = { -1, -1 };

    default_style = default_menu.style;

    for (u32 i = 0; i < IN_GAME; i++) {
        game->menu_list.menus[i] = default_menu;
    }

    // Online
    game->mutex = win32_create_mutex();
    online.mutex = win32_create_mutex();
    game->selected_mutex = win32_create_mutex();

    Descriptor texture_desc = render_get_descriptor_set_index(&layouts[2], 0);

    for (u32 j = 0; j < 14; j++) {
        game->indices[j] = render_set_bitmap(&texture_desc, &card_bitmaps[j]);
    }
    game->indices[14] = render_set_bitmap(&texture_desc, find_bitmap(&game->assets, "YOGI"));
    Model *model = find_model(&game->assets, "TABLE");
    game->indices[15] = render_set_bitmap(&texture_desc, &model->meshes[0].material.diffuse_map);
    
    init_triangles(find_model(&game->assets, "CARD"));
    
	return false;
}

internal void
prepare_controller_for_input(Controller *controller) {
    for (u32 j = 0; j < ARRAY_COUNT(controller->buttons); j++)
        controller->buttons[j].previous_state = controller->buttons[j].current_state;
}

internal void
controller_process_input(Controller *controller, s32 id, bool8 state) {
    for (u32 i = 0; i < ARRAY_COUNT(controller->buttons); i++) {
        // loop through all ids associated with button
        for (u32 j = 0; j < controller->buttons[i].num_of_ids; j++) {
            if (id == controller->buttons[i].ids[j]) controller->buttons[i].current_state = state;
        }
    }
}

bool8 update(App *app) {
	State *state = (State *)app->data;
	Assets *assets = &state->assets;

    bool8 select = on_up(state->controller.select) || on_up(state->controller.mouse_left);
    bool8 pressed = on_down(state->controller.select) || on_down(state->controller.mouse_left);
    Menu_Input menu_input = {
        select,
        pressed,
        app->input.active,

        state->controller.forward,
        state->controller.backward,
        state->controller.left,
        state->controller.right,

        { 0, 0 },
        0,

        app->input.mouse,

        app->input.buffer,
        app->input.buffer_index
    };

    if (app->window.resized) {

    }
    
    // Update
    if (state->is_client)
        client_get_game(state->client, state);

    if (state->menu_list.mode == IN_GAME) {
        update_game(state, app);

        //if (state->is_client)
            //client_set_game(state->client, &state->game);
    } else if (state->menu_list.mode == PAUSE_MENU) {
        if (on_down(state->controller.pause)) {
            state->menu_list.mode = IN_GAME;
        }
    }

    if (state->is_server)
        win32_wait_mutex(state->mutex);

    // Draw
    Shader *basic_3D = find_shader(assets, "BASIC3D");
    Shader *color_3D = find_shader(assets, "COLOR3D");

    for (u32 i = 0; i < 10; i++) {
        layouts[i].reset();
    }
    render_start_frame();

    texture_desc = render_get_descriptor_set_index(&layouts[2], 0);

    render_set_viewport(app->window.width, app->window.height);
    render_set_scissor(0, 0, app->window.width, app->window.height);

    state->scene_set = render_get_descriptor_set(&layouts[0]);
    //state->scene.view = look_at({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
    state->scene.projection = perspective_projection(45.0f, (float32)app->window.width / (float32)app->window.height, 0.1f, 1000.0f);
    render_update_ubo(state->scene_set, (void*)&state->scene);

    state->scene_ortho_set = render_get_descriptor_set(&layouts[0]);
    state->ortho_scene.view = identity_m4x4();
    state->ortho_scene.projection = orthographic_projection(0.0f, (float32)app->window.width, 0.0f, (float32)app->window.height, -3.0f, 3.0f);
    render_update_ubo(state->scene_ortho_set, (void*)&state->ortho_scene);
    
    render_depth_test(false);

    light_set = render_get_descriptor_set(&layouts[1]);
    render_update_ubo(light_set, (void*)&global_light);

    light_set_2 = render_get_descriptor_set(&layouts[1]);
    render_update_ubo(light_set_2, (void*)&global_light_2);

    if (state->menu_list.mode != IN_GAME && state->menu_list.mode != PAUSE_MENU) {
        render_bind_pipeline(&shapes.color_pipeline);
        //render_bind_descriptor_set(state->scene_ortho_set, 0);
        render_bind_descriptor_set(state->scene_ortho_set);
    }

    switch(state->menu_list.mode) {
        case MAIN_MENU: {
            if (draw_main_menu(state, &state->menu_list.main, &menu_input, app->window.dim))
                return 1;
        } break;

        case LOCAL_MENU: {
            draw_local_menu(state, &state->menu_list.local, &menu_input, app->window.dim);
        } break;

        case SCOREBOARD_MENU: {
            s32 sb_result = draw_scoreboard(&state->menu_list.scoreboard, &state->game, &menu_input, app->window.dim);
            if (sb_result == 1) {
                state->menu_list.mode = IN_GAME;
            } else if (sb_result == 2) {
                state->menu_list.mode = MAIN_MENU;
                if (state->is_server) {
                    state->is_server = false;
                    close_server();
                }
            }
        } break;

        case HOST_MENU: {
            draw_host_menu(&state->menu_list.host, state, &menu_input, app->window.dim);
        } break;

        case JOIN_MENU: {
            draw_join_menu(&state->menu_list.join, state, &menu_input, app->window.dim);
        } break;

        case PAUSE_MENU:
        case IN_GAME: {
            render_depth_test(true);
            
            render_bind_pipeline(&basic_pipeline);

            render_bind_descriptor_set(state->scene_set);
            render_bind_descriptor_set(light_set);
            render_bind_descriptor_set(texture_desc);

            draw_game(state, &state->assets, basic_3D, &state->game, false, state->indices);
            render_bind_pipeline(&color_pipeline);
            //draw_sphere({ 0, 0, 0 }, 0.0f, { 1, 1, 1 }, { 0, 255, 0, 1 });
            render_depth_test(false);

            if (state->game.round_type != HOLE_OVER)
                draw_game(state, &state->assets, basic_3D, &state->game, true, state->indices);

            //draw_ray(&state->mouse_ray);

            render_bind_descriptor_set(state->scene_ortho_set);

            float32 pixel_height = app->window.dim.x / 20.0f;
            Vector2 text_dim = get_string_dim(default_font, state->game.players[state->game.active_player].name, pixel_height, { 255, 255, 255, 1 });
            draw_string_tl(default_font, state->game.players[state->game.active_player].name, { app->window.dim.x - text_dim.x - 5, 5 }, pixel_height, { 255, 255, 255, 1 });

            draw_string_tl(find_font(&state->assets, "CASLON"), round_types[state->game.round_type], { 5, 5 }, pixel_height, { 255, 255, 255, 1 });

            Player *active_player = &state->game.players[state->game.active_player];
            if (state->menu_list.mode == PAUSE_MENU) {
                draw_pause_menu(state, &state->menu_list.pause, &menu_input, app->window.dim);
            } else if (state->game.round_type == HOLE_OVER) {

                if (!state->is_client) {                
                    Button_Input button_input = {
                        app->input.active,
                        state->controller.select,
                        app->input.mouse,
                        state->controller.mouse_left
                    };
                    float32 pass_width = app->window.dim.x / 7.0f;
                    if (gui_button(&gui, default_style, "Proceed", default_font, { app->window.dim.x - pass_width, 5 + text_dim.y + 50 }, { pass_width, pixel_height }, button_input)) {
                        state->menu_list.mode = SCOREBOARD_MENU;
                        state->menu_list.scoreboard.initialized = false;
                    }
                }

            } else if (active_player->turn_stage == FLIP_CARD && get_number_flipped(active_player->flipped) == HAND_SIZE - 1) {

                Button_Input button_input = {
                    app->input.active,
                    state->controller.select,
                    app->input.mouse,
                    state->controller.mouse_left
                };
                float32 pass_width = app->window.dim.x / 7.0f;
                if (gui_button(&gui, default_style, "Pass", default_font, { app->window.dim.x - pass_width, 5 + text_dim.y + 50 }, { pass_width, pixel_height }, button_input)) {
                    state->pass_selected = true; // in update_game feed this into selected
                }
            }

        } break;
    }

    draw_onscreen_notifications(&state->notifications, app->window.dim, (float32)app->time.frame_time_s);

    char buffer[20];
    float_to_char_array((float32)app->time.frames_per_s, buffer, 20);
    draw_string_tl(default_font, buffer, { 10, 40 }, 40.0f, { 255, 50, 50, 1 });


    render_end_frame();

    if (state->is_server)
        win32_release_mutex(state->mutex);

    prepare_controller_for_input(&state->controller);
    gui.index = 1;
    
	return 0;
}

bool8 test(App *app) {
    return 0;
}

s32 event_handler(App *app, App_System_Event event, u32 arg) {
    State *state = (State *)app->data;

    switch(event) {
        case APP_INIT: {
            app->update = &update;
            if (init_data(app))
                return 1;
        } break;

        case APP_EXIT: {
            if (state->is_client)
                client_close_connection(state->client);
            if (state->is_server)
                close_server();

            cleanup_shapes();
            render_assets_cleanup(&state->assets);
        } break;

        case APP_KEYDOWN: {
            app->input.active = KEYBOARD_INPUT;
            controller_process_input(&state->controller, arg, true);
        } break;

        case APP_KEYUP: {
            app->input.active = KEYBOARD_INPUT;
            controller_process_input(&state->controller, arg, false);
        } break;

        case APP_MOUSEDOWN: {
            controller_process_input(&state->controller, arg, true);
        } break;

        case APP_MOUSEUP: {
            controller_process_input(&state->controller, arg, false);
        } break;
    }

    return 0;
}

