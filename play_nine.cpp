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
#include "raytrace.h"
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

internal void
copy_bitmap_into_bitmap(Bitmap dest, const Bitmap src, Vector2_s32 position) {
    u8 *dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch);
    u8 *src_ptr = src.memory;
    for (s32 y = 0; y < src.height; y++) {
        for (s32 x = 0; x < src.width; x++) {   
            u8 color = 0xFF ^ (*src_ptr);

            for (s32 i = 0; i < dest.channels; i++) {
                for (s32 j = 0; j < src.channels; j++) {
                    dest_ptr[i] = src_ptr[j];
                }
            }

            dest_ptr += dest.channels;
            src_ptr += src.channels;
        }   

        dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch) + (y * dest.pitch);
    }
}

internal Vector4
get_color(Vector4 c1, Vector4 c2) {
    float32 alpha = 255 - ((255 - c1.E[3]) * (255 - c2.E[3]) / 255);
    float32 red   = (c1.E[0] * (255 - c2.E[3]) + c2.E[0] * c2.E[3]) / 255;
    float32 green = (c1.E[1] * (255 - c2.E[3]) + c2.E[1] * c2.E[3]) / 255;
    float32 blue  = (c1.E[2] * (255 - c2.E[3]) + c2.E[2] * c2.E[3]) / 255;
    return {red, green, blue, alpha};
}

internal void
copy_blend_bitmap(Bitmap dest, const Bitmap src, Vector2_s32 position) {
    u8 *dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch);
    u8 *src_ptr = src.memory;
    for (s32 y = 0; y < src.height; y++) {
        for (s32 x = 0; x < src.width; x++) {   
            Vector4 src_color = { 0x00, 0x00, 0x00, float32(*src_ptr) };
            Vector4 dest_color = { float32(dest_ptr[0]), float32(dest_ptr[1]), float32(dest_ptr[2]), float32(dest_ptr[3]) };
            Vector4 blend_color = get_color(dest_color, src_color);

            dest_ptr[0] = (u8)blend_color.r;
            dest_ptr[1] = (u8)blend_color.g;
            dest_ptr[2] = (u8)blend_color.b;
            dest_ptr[3] = (u8)blend_color.a;

            dest_ptr += dest.channels;
            src_ptr += src.channels;
        }   

        dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch) + (y * dest.pitch);

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

        copy_bitmap_into_bitmap(bitmap, fbitmap->bitmap, cv2(char_coords));

        s32 kern = get_codepoint_kern_advance(font->info, str[i], str[i + 1]);
        current_point += scale * (kern + font_char->ax);
        
        i++;
    }

    return bitmap;
}

internal Bitmap
create_card_bitmap(Font *font, s32 number) {
    Bitmap bitmap   = {};
    bitmap.width    = 1024;
    bitmap.height   = 1638;
    bitmap.channels = 4;
    bitmap.pitch    = bitmap.width * bitmap.channels;
    bitmap.memory   = (u8*)platform_malloc(bitmap.width * bitmap.height * bitmap.channels);
    memset(bitmap.memory, 0xFF, bitmap.width * bitmap.height * bitmap.channels);

    char str[3] = {};
    switch(number) {
        case -5: str[0] = '-'; str[1] = '5'; break;
        case 10: str[0] = '1'; str[1] = '0'; break;
        case 11: str[0] = '1'; str[1] = '1'; break;
        case 12: str[0] = '1'; str[1] = '2'; break;
        default: str[0] = number + 48; break;
    }

    Bitmap str_bitmap = create_string_into_bitmap(font, 500.0f, str);

    u32 x_center = (bitmap.width  / 2) - (str_bitmap.width  / 2);
    u32 y_center = (bitmap.height / 2) - (str_bitmap.height / 2);
    copy_blend_bitmap(bitmap, str_bitmap, { s32(x_center), s32(y_center) });

    render_create_texture(&bitmap, TEXTURE_PARAMETERS_CHAR);

    platform_free(str_bitmap.memory);
    platform_free(bitmap.memory);
  
    return bitmap;
}

internal void
init_card_bitmaps(Bitmap *bitmaps, Font *font) {
    for (s32 i = 0; i <= 12; i++) {
        bitmaps[i] = create_card_bitmap(font, i);
    }

    bitmaps[13] = create_card_bitmap(font, -5);
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
            rot->degrees += seconds * rot->speed;
        } else {
            rot->rotating = false;
        }
    } else if (rot->speed < 0.0f) { 
        if (rot->degrees > rot->dest_degrees) {
            rot->degrees += seconds * rot->speed;
        } else {
            rot->rotating = false;
        }
    }

    return rot->degrees;
}

internal void
load_card_model(Card *card, bool8 flipped, Vector3 position, float32 degrees, Vector3 scale) {
    Quaternion rotation = get_rotation(degrees * DEG2RAD, {0, 1, 0});
    if (!flipped) {
        Quaternion flip = get_rotation(180.0f * DEG2RAD, {0, 0, 1});
        rotation = flip * rotation;
        position.y += 0.05f;
    }

    card->model = create_transform_m4x4(position, rotation, scale);
}

internal void
load_card_models(Game *game, Game_Draw *draw, float32 seconds) {
    Vector3 card_scale           = {1.0f, 0.5f, 1.0f};
    Vector3 selected_card_coords = {0.0f, 1.0f, -3.1f};
    Vector3 pile_coords          = { -1.1f, 0.35f, 0 };
    Vector3 discard_pile_coords  = { 1.1f, 0, 0 };

    for (u32 i = 0; i < game->num_of_players; i++) {
        float32 degrees = (draw->degrees_between_players * i) - 90.0f;
        float32 rad = degrees * DEG2RAD; 
        Vector3 position = get_hand_position(draw->radius, draw->degrees_between_players, i);
        
        // draw player cards    
        for (u32 card_index = 0; card_index < 8; card_index++) {
            Vector3 card_pos = { hand_coords[card_index].x, 0.0f, hand_coords[card_index].y };
            rotate_coords(&card_pos, rad);
            card_pos += position;
            load_card_model(&deck[game->players[i].cards[card_index]], game->players[i].flipped[card_index], card_pos, degrees, card_scale);
        }
    }

    // draw card selected from pile
    float32 degrees = (draw->degrees_between_players * game->active_player) - 90.0f;
    float32 rad = degrees * DEG2RAD; 
    if (game->players[game->active_player].turn_stage == SELECT_CARD) {
        rotate_coords(&selected_card_coords, rad);
        load_card_model(&deck[game->players[game->active_player].new_card], true, selected_card_coords, degrees, card_scale);
    }

    float32 pile_degs = degrees - process_rotation(&draw->pile_rotation, seconds);
    float32 pile_rad = pile_degs * DEG2RAD;    

    rotate_coords(&pile_coords, pile_rad);
    rotate_coords(&discard_pile_coords, pile_rad);

    // pile
    load_card_model(&deck[game->pile[game->top_of_pile]], false, pile_coords, pile_degs, {1.0f, 4.0f, 1.0f});

    // discard pile
    if (game->top_of_discard_pile != 0)
        load_card_model(&deck[game->discard_pile[game->top_of_discard_pile - 1]], true, discard_pile_coords, pile_degs, card_scale);
}

internal void
draw_card(Assets *assets, Shader *shader, Card card) {
    u32 bitmap_index = card.number;
    if (card.number == -5)
        bitmap_index = 13;

    render_draw_model(find_model(assets, "CARD"), &color_pipeline, &basic_pipeline, &card_bitmaps[bitmap_index], find_bitmap(assets, "YOGI"), card.model);
} 

//      180
// 270        90
//       0
internal void
draw_game(Assets *assets, Shader *shader, Game *game) {
    render_bind_pipeline(&basic_pipeline);
    render_draw_model(find_model(assets, "TABLE"), basic_pipeline.shader, { 0, -1.05f, 0 }, get_rotation(0, { 0, 1, 0 }));

    for (u32 i = 0; i < game->num_of_players; i++) {
        for (u32 card_index = 0; card_index < 8; card_index++) {
            Card card = deck[game->players[i].cards[card_index]];
            draw_card(assets, shader, card);
        }
    }

    if (game->players[game->active_player].turn_stage == SELECT_CARD) {
        Card card = deck[game->players[game->active_player].new_card];
        draw_card(assets, shader, card);
    }

    draw_card(assets, shader, deck[game->pile[game->top_of_pile]]);

    if (game->top_of_discard_pile != 0)
        draw_card(assets, shader, deck[game->discard_pile[game->top_of_discard_pile - 1]]);
}

internal void
init_deck() {
    u32 deck_index = 0;
    for (s32 i = 0; i <= 12; i++) {
        for (u32 j = deck_index; j < deck_index + 8; j++) {
            deck[j].number = i;
        }
        deck_index += 8;
    }

    for (u32 j = deck_index; j < deck_index + 4; j++) {
        deck[j].number = -5;
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
/*
    game->top_of_pile = 0;
    game->top_of_discard_pile = 0;
    shuffle_pile(game->pile);
    game->num_of_players = num_of_players;
    deal_cards(game);
    game->active_player = 0;
    game->round_type = FLIP_ROUND;
    game->last_turn = 0;
*/
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
    Card top_card = deck[top];
    Card bottom_card = deck[bottom];

    if (top_card.number == bottom_card.number) {
        result.matching = true;
        result.number = top_card.number; // matching so top and bottom same number
    }

    if (!result.matching || (top_card.number == -5))
        result.score += top_card.number + bottom_card.number;

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
        print("Player %d: %d\n", i, score);
        player->scores[game->holes_played] += score;
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
        for (u32 i = 0; i < 8; i++) {
            game->players[game->active_player].flipped[i] = true;
        }
    }

    game->active_player++;
    
    // END HOLE
    if (game->last_turn == game->num_of_players) {
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

                //deck[active_player->new_card].flipped = true;
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
                    active_player->turn_stage = SELECT_PILE;
                    active_player->pile_card = false;
                    if (get_number_flipped(active_player->flipped) == HAND_SIZE)
                        game->round_type = FINAL_ROUND;
                    next_player(game, draw);
                    return;
                }
            }

            // discard pile only emtpy if picked from and you can't put it back after you pick it up
            // no reason not to pick up from the regular pile
            if (game->top_of_discard_pile == 0)
                return;

            if (selected[DISCARD_PILE] && active_player->pile_card) {
                game->discard_pile[game->top_of_discard_pile++] = active_player->new_card;
                active_player->new_card = 0;
                active_player->turn_stage = FLIP_CARD;
            }
        } break;

        case FLIP_CARD: {
            for (u32 i = 0; i < HAND_SIZE; i++) {
                Card *card = &deck[active_player->cards[i]];
                if (selected[i] && !active_player->flipped[i]) {
                    //card->flipped = true;
                    active_player->flipped[i] = true;
                    active_player->turn_stage = SELECT_PILE;
                    active_player->pile_card = false;
                    if (get_number_flipped(active_player->flipped) == HAND_SIZE) {
                        game->round_type = FINAL_ROUND; 
                        game->last_turn = 0;
                    }
                    next_player(game, draw);
                    return;
                }

                if (selected[DISCARD_PILE] && get_number_flipped(active_player->flipped) == HAND_SIZE - 1) {
                    active_player->turn_stage = SELECT_PILE;
                    active_player->pile_card = false;
                    next_player(game, draw);
                }
            }
        } break;
    }
}

internal bool8
ray_model_intersection(Ray ray, Model *model, Card *card) {
    for (u32 i = 0; i < model->meshes_count; i++) {
        Ray_Intersection p = intersect_triangle_mesh(ray, &model->meshes[i], card->model);
        if (p.number_of_intersections != 0) {
            //print("card: %f %f %f\n", p.point.x, p.point.y, p.point.z);
            return true;
        }
    }

    return false;
}

internal void
mouse_ray_model_intersections(bool8 *selected, Ray mouse_ray, Game *game, Model *card_model) {
    Player *active_player = &game->players[game->active_player];

    for (u32 card_index = 0; card_index < 8; card_index++) {
        selected[card_index] = ray_model_intersection(mouse_ray, card_model, &deck[active_player->cards[card_index]]);
    }

    selected[PICKUP_PILE] = ray_model_intersection(mouse_ray, card_model, &deck[game->pile[game->top_of_pile]]);
    selected[DISCARD_PILE] = ray_model_intersection(mouse_ray, card_model, &deck[game->discard_pile[game->top_of_discard_pile - 1]]);
}

internal void
update_card_with_buttons(bool8 selected[SELECTED_SIZE], Game *game, Controller *controller) {
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

    load_card_models(game, draw, app->time.frame_time_s);

    switch(state->camera_mode) {
        case FREE_CAMERA: {
            if (on_down(state->controller.pause)) {
                state->camera_mode = PLAYER_CAMERA;
                app->input.relative_mouse_mode = false;
            }

            if (app->input.relative_mouse_mode) {
                float32 mouse_m_per_s = 100.0f;
                float32 mouse_move_speed = mouse_m_per_s * app->time.frame_time_s;
                update_camera_with_mouse(&state->camera, app->input.mouse_rel, { mouse_move_speed, mouse_move_speed });
                update_camera_target(&state->camera);    
                state->scene.view = get_view(state->camera);
                render_update_ubo(state->scene_set, 0, (void*)&state->scene, true);

                float32 m_per_s = 6.0f; 
                float32 m_moved = m_per_s * app->time.frame_time_s;
                Vector3 move_vector = {m_moved, m_moved, m_moved};
                update_camera_with_keys(&state->camera, state->camera.target, state->camera.up, move_vector,
                                        is_down(state->controller.forward),  is_down(state->controller.backward),
                                        is_down(state->controller.left),  is_down(state->controller.right),
                                        is_down(state->controller.up),  is_down(state->controller.down));                             

                //print("%f %f %f\n", state->camera.position.x, state->camera.position.y, state->camera.position.z);
            }
        } break;

        case PLAYER_CAMERA: {
            if (on_down(state->controller.pause)) {
                //state->camera_mode = FREE_CAMERA;
                state->menu_list.mode = PAUSE_MENU;
                //app->input.relative_mouse_mode = true;
            }
                  
            if (on_down(state->controller.right)) {
                next_player(game, &state->game_draw);
            }

            float32 cam_dis = 8.0f + draw->radius;
            float32 deg = -draw->degrees_between_players * game->active_player;
            deg += process_rotation(&draw->camera_rotation, app->time.frame_time_s);
            float32 rad = deg * DEG2RAD;
            float32 x = cam_dis * cosf(rad);
            float32 y = cam_dis * sinf(rad);

            state->camera.position =  Vector3{ x, 14.0f, y };
            state->camera.yaw      = deg + 180.0f;
            state->camera.pitch    = -50.0f;

            update_camera_target(&state->camera);    
            state->scene.view = get_view(state->camera);
            render_update_ubo(state->scene_set, 0, (void*)&state->scene, true);

            bool8 selected[SELECTED_SIZE] = {};
            if (state->client_game_index == game->active_player || (!state->is_client && !state->is_server)) {
                if (on_up(state->controller.mouse_left)) {
                    set_ray_coords(&state->mouse_ray, state->camera, state->scene.projection, state->scene.view, app->input.mouse, app->window.dim);
                     
                    Model *card_model = find_model(&state->assets, "CARD");
                    mouse_ray_model_intersections(selected, state->mouse_ray, game, card_model);
                }

                update_card_with_buttons(selected, game, &state->controller);

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

            // game logic
            switch(game->round_type) {
                case FLIP_ROUND:    flip_round_update(game, &state->game_draw, selected); break;
                case FINAL_ROUND:
                case REGULAR_ROUND: regular_round_update(game, &state->game_draw, selected); break;
            }
        } break;
    }

    return 0;
}


bool8 init_data(App *app) {
	app->data = platform_malloc(sizeof(State));
	platform_memory_set(app->data, 0, sizeof(State));
    State *game = (State *)app->data;
	game->assets = {};
	if (load_assets(&game->assets, "../assets.ethan"))
        return true;

	Shader *basic_3D = find_shader(&game->assets, "BASIC3D");
    render_compile_shader(basic_3D);
    init_basic_vert_layout(basic_3D);
    init_basic_frag_layout(basic_3D);
    basic_pipeline.shader = basic_3D;
    basic_pipeline.depth_test = true;
	render_create_graphics_pipeline(&basic_pipeline, get_vertex_xnu_info());
	
    Shader *color_3D = find_shader(&game->assets, "COLOR3D");
    render_compile_shader(color_3D);
    init_basic_vert_layout(color_3D);
    init_color3D_frag_layout(color_3D);
    color_pipeline.shader = color_3D;
    color_pipeline.depth_test = true;
    render_create_graphics_pipeline(&color_pipeline, get_vertex_xnu_info());

	// Init assets
	Bitmap *yogi = find_bitmap(&game->assets, "YOGI");
    render_create_texture(yogi, TEXTURE_PARAMETERS_DEFAULT);

	//Bitmap *david = find_bitmap(&game->assets, "DAVID");
    //render_create_texture(david, TEXTURE_PARAMETERS_DEFAULT);
	
	Font *font = find_font(&game->assets, "CASLON");
    init_font(font);

	//render_init_model(find_model(&game->assets, "TAILS"));
    render_init_model(find_model(&game->assets, "CARD"));
    render_init_model(find_model(&game->assets, "TABLE"));

	// Rendering
	//game->scene.view = look_at({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
    game->scene.projection = perspective_projection(45.0f, (float32)app->window.width / (float32)app->window.height, 0.1f, 1000.0f);

    // Init ubo
    game->scene_set = render_get_descriptor_set(basic_3D, 0);
    render_update_ubo(game->scene_set, 0, (void*)&game->scene, true);

    game->scene_ortho_set = render_get_descriptor_set(basic_3D, 0);
    game->ortho_scene.view = identity_m4x4();
    game->ortho_scene.projection = orthographic_projection(0.0f, (float32)app->window.width, 0.0f, (float32)app->window.height, -3.0f, 3.0f);
    render_update_ubo(game->scene_ortho_set, 0, (void*)&game->ortho_scene, true);

    game->camera.position = { 0, 2, -5 };
    game->camera.target   = { 0, 0, 0 };
    game->camera.up       = { 0, -1, 0 };
    game->camera.fov      = 75.0f;
    game->camera.yaw      = 180.0f;
    game->camera.pitch    = -75.0f;
		
	set(&game->controller.forward, 'w');
	set(&game->controller.backward, SDLK_s);
	set(&game->controller.left, SDLK_a);
	set(&game->controller.right, SDLK_d);
	set(&game->controller.up, SDLK_SPACE);
	set(&game->controller.down, SDLK_LSHIFT);

    set(&game->controller.select, SDLK_RETURN);
	set(&game->controller.pause,  SDLK_ESCAPE);

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
    init_card_bitmaps(card_bitmaps, font);

    clear_font_bitmap_cache(font);
    
    init_shapes(&game->assets);

    game->game_draw.rotation_speed = 150.0f;
    game->camera_mode = PLAYER_CAMERA;

    game->notifications.font = font;
    game->notifications.text_color = { 255, 255, 255, 1 };

    default_font = find_font(&game->assets, "CASLON");

    // Setting default Menus
    Menu default_menu = {};
    default_menu.font = default_font;
    default_menu.button_style.default_back_color = { 231, 213, 36,  1 };
    default_menu.button_style.active_back_color  = { 240, 229, 118, 1 };
    default_menu.button_style.default_text_color = { 39,  77,  20,  1 };
    default_menu.button_style.active_text_color  = { 39,  77,  20,  1 };

    default_menu.style.default_back = { 231, 213, 36,  1 };
    default_menu.style.default_text = { 39,  77,  20,  1 };
    default_menu.style.hot_back     = { 240, 229, 118, 1 };
    default_menu.style.hot_text     = { 39,  77,  20,  1 };
    default_menu.style.active_back  = { 231, 213, 36,  1 };
    default_menu.style.active_text  = { 39,  77,  20,  1 };

    default_menu.active_section = { -1, -1 };

    for (u32 i = 0; i < IN_GAME; i++) {
        game->menu_list.menus[i] = default_menu;
    }

    // Online
    game->mutex = win32_create_mutex();
    online.mutex = win32_create_mutex();
    game->selected_mutex = win32_create_mutex();

    platform_memory_copy(game->name, "Jeff", 5);
    platform_memory_copy(game->ip, "127.0.0.1", 10);
    platform_memory_copy(game->port, "4444", 5);

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
    Menu_Input menu_input = {
        select,
        app->input.active,

        state->controller.forward,
        state->controller.backward,
        state->controller.left,
        state->controller.right,

        { 0, 0 },
        0,

        { 0, 0 },
        0,

        app->input.mouse,

        app->input.buffer,
        app->input.buffer_index
    };

    if (app->window.resized) {
        state->scene.projection = perspective_projection(45.0f, (float32)app->window.width / (float32)app->window.height, 0.1f, 1000.0f);
        render_update_ubo(state->scene_set, 0, (void*)&state->scene, true);

        state->ortho_scene.projection = orthographic_projection(0.0f, (float32)app->window.width, 0.0f, (float32)app->window.height, -3.0f, 3.0f);
        render_update_ubo(state->scene_ortho_set, 0, (void*)&state->ortho_scene, true);
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

    vulkan_reset_descriptor_sets(assets);
    render_start_frame();

    render_set_viewport(app->window.width, app->window.height);
    render_set_scissor(app->window.width, app->window.height);

    switch(state->menu_list.mode) {
    case MAIN_MENU: {
        render_bind_pipeline(&shapes.color_pipeline);
        render_bind_descriptor_set(state->scene_ortho_set, 0);
        if (draw_main_menu(state, &state->menu_list.main, &menu_input, app->window.dim))
            return 1;
    } break;

    case LOCAL_MENU: {
        render_bind_pipeline(&shapes.color_pipeline);
        render_bind_descriptor_set(state->scene_ortho_set, 0);
        draw_local_menu(state, &state->menu_list.local, &menu_input, app->window.dim);
    } break;

    case SCOREBOARD_MENU: {
        render_bind_pipeline(&shapes.color_pipeline);
        render_bind_descriptor_set(state->scene_ortho_set, 0);
        s32 sb_result = draw_scoreboard(&state->menu_list.scoreboard, &state->game, &menu_input, app->window.dim);
        if (sb_result == 1) {
            state->menu_list.mode = IN_GAME;
        } else if (sb_result == 2) {
            state->menu_list.mode = MAIN_MENU;
        }
    } break;

    case HOST_MENU: {
        render_bind_pipeline(&shapes.color_pipeline);
        render_bind_descriptor_set(state->scene_ortho_set, 0);
        draw_host_menu(&state->menu_list.host, state, &menu_input, app->window.dim);
    } break;

    case JOIN_MENU: {
        render_bind_pipeline(&shapes.color_pipeline);
        render_bind_descriptor_set(state->scene_ortho_set, 0);
        draw_join_menu(&state->menu_list.join, state, &menu_input, app->window.dim);
    } break;

    case PAUSE_MENU:
    case IN_GAME: {
        render_bind_pipeline(&color_pipeline);
        render_bind_descriptor_set(state->scene_set, 0);

        draw_game(&state->assets, basic_3D, &state->game);

        //draw_ray(&state->mouse_ray);

        render_bind_descriptor_set(state->scene_ortho_set, 0);

        float32 pixel_height = app->window.dim.x / 20.0f;
        Vector2 text_dim = get_string_dim(default_font, state->game.players[state->game.active_player].name, pixel_height, { 255, 255, 255, 1 });
        draw_string_tl(default_font, state->game.players[state->game.active_player].name, { app->window.dim.x - text_dim.x - 5, 5 }, pixel_height, { 255, 255, 255, 1 });

        draw_string_tl(find_font(&state->assets, "CASLON"), round_types[state->game.round_type], { 5, 5 }, pixel_height, { 255, 255, 255, 1 });

        if (state->menu_list.mode == PAUSE_MENU) {
            draw_pause_menu(state, &state->menu_list.pause, &menu_input, app->window.dim);
        } else if (state->game.round_type == HOLE_OVER) {
            draw_hole_over_menu(state, &state->menu_list.pause, &menu_input, app->window.dim);
        } 
    } break;
    }

    draw_onscreen_notifications(&state->notifications, app->window.dim, app->time.frame_time_s);

    render_end_frame();

    if (state->is_server)
        win32_release_mutex(state->mutex);

    prepare_controller_for_input(&state->controller);

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

            for (u32 i = 0; i < 14; i++) {
                vulkan_delete_texture(&card_bitmaps[i]);
            }
            cleanup_shapes();
            assets_cleanup(&state->assets);
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

