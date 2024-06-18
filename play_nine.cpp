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

#define PICKUP_PILE    8
#define DISCARD_PILE   9
#define PASS_BUTTON   10
#define SELECTED_SIZE 11

enum Game_Input {
    GI_0,
    GI_1,
    GI_2,
    GI_3,

    GI_4,
    GI_5,
    GI_6,
    GI_7,

    GI_PICKUP_PILE,
    GI_DISCARD_PILE,

    GI_PASS_BUTTON,

    GI_SIZE
};

#define MAX_NAME_SIZE  20
#define MAX_PLAYERS     6
#define HAND_SIZE       8
#define DECK_SIZE     108
#define MAX_HOLES      20 // max holes that can be played in one gamed
#define GAME_LENGTH     2 // play NINE

#define DRAW_SIGNALS_AMOUNT 6

#include "application.h"
#include "play_nine_input.h"
#include "play_nine_draw.h"
#include "play_nine.h"
#include "play_nine_online.h"

internal void next_player(Game *game);

#include "play_nine_raytrace.cpp"
#include "play_nine_init.cpp"
#include "play_nine_score.cpp"
#include "play_nine_online.cpp"
#include "play_nine_bitmaps.cpp"
#include "play_nine_menus.cpp"
#include "play_nine_draw.cpp"
#include "play_nine_bot.cpp"

//
// game logic
//

internal void
next_player(Game *game) {
    // Flip all cards after final turn
    if (game->round_type == FINAL_ROUND) {
        game->last_turn++;
        for (u32 i = 0; i < HAND_SIZE; i++) {
            if (!game->players[game->active_player].flipped[i]) {
                game->players[game->active_player].flipped[i] = true;
                add_draw_signal(draw_signals, SIGNAL_FLIP_CARD, i, game->active_player);
            }
        }
    }

    // END HOLE
    if (game->last_turn == game->num_of_players && game->round_type != HOLE_OVER) {
        update_scores(game);
        game->round_type = HOLE_OVER;
        game->turn_stage = FLIP_CARD;

        increment_player(&game->starting_player, game->num_of_players);

        // END GAME
        game->holes_played++;
        if (game->holes_played == GAME_LENGTH) {
            game->game_over = true;
        }

        return; // Don't need to move ahead a player now
    }

    game->pile_card = false;
    game->turn_stage = SELECT_PILE;
    increment_player(&game->active_player, game->num_of_players);

    if (game->active_player == game->starting_player && game->round_type == FLIP_ROUND) {
        game->round_type = REGULAR_ROUND;
    }

    if (game->round_type == FLIP_ROUND) {
        game->turn_stage = FLIP_CARD;
    }

    if (game->players[game->active_player].is_bot)
        game->bot_thinking_time = 0.0f;

    game->turn_time = 0.0f;

    add_draw_signal(draw_signals, SIGNAL_NEXT_PLAYER_ROTATION);
}

internal void
do_update_with_input(Game *game, bool8 selected[SELECTED_SIZE]) {
    Player *active_player = &game->players[game->active_player];
    switch(game->turn_stage) {
        case SELECT_PILE: {
            if (selected[PICKUP_PILE]) {
                add_draw_signal(draw_signals, SIGNAL_NEW_CARD_FROM_PILE);
                game->new_card = game->pile[game->top_of_pile++];

                // @SPECIAL case
                // Probably shouldn't happen in a real game
                if (game->top_of_pile + game->num_of_players >= DECK_SIZE) {
                    game->round_type = FINAL_ROUND;
                }

                game->turn_stage = SELECT_CARD;
                game->pile_card = true;
            } else if (selected[DISCARD_PILE]) {
                add_draw_signal(draw_signals, SIGNAL_NEW_CARD_FROM_DISCARD);
                game->new_card = game->discard_pile[game->top_of_discard_pile - 1];
                game->top_of_discard_pile--;
                game->turn_stage = SELECT_CARD;
            }
        } break;

        case SELECT_CARD: {
            for (u32 i = 0; i < HAND_SIZE; i++) {
                if (selected[i]) {
                    game->discard_pile[game->top_of_discard_pile++] = active_player->cards[i];
                    add_draw_signal(draw_signals, SIGNAL_REPLACE, i, game->active_player, active_player->flipped[i]);
                    active_player->flipped[i] = true;
                    active_player->cards[i] = game->new_card;
                    game->new_card = 0;

                    if (get_number_flipped(active_player->flipped) == HAND_SIZE)
                        game->round_type = FINAL_ROUND;
                    next_player(game);
                    return;
                }
            }

            if (game->pile_card && selected[DISCARD_PILE]) {
                game->discard_pile[game->top_of_discard_pile++] = game->new_card;
                game->new_card = 0;
                game->turn_stage = FLIP_CARD;
                add_draw_signal(draw_signals, SIGNAL_DISCARD_SELECTED);
            }
        } break;

        case FLIP_CARD: {
            for (u32 i = 0; i < HAND_SIZE; i++) {
                if (selected[i] && !active_player->flipped[i]) {    
                    add_draw_signal(draw_signals, SIGNAL_FLIP_CARD, i, game->active_player);
                    active_player->flipped[i] = true;

                    if (game->round_type == FLIP_ROUND) {
                        // check if player turn over
                        if (get_number_flipped(active_player->flipped) == 2) {
                            next_player(game);
                            return;
                        }
                    } else {                
                        if (get_number_flipped(active_player->flipped) == HAND_SIZE && game->round_type != FINAL_ROUND) {
                            game->round_type = FINAL_ROUND; 
                            game->last_turn = 0;
                        }
                        next_player(game);
                        return;
                    }
                }

                if (selected[PASS_BUTTON] && get_number_flipped(active_player->flipped) == HAND_SIZE - 1) {
                    next_player(game);
                    return;
                }

            }
        } break;
    }
}

internal void
do_mouse_selected_update(State *state, App *app, bool8 selected[SELECTED_SIZE]) {
    Game *game = &state->game;
    Game_Draw *draw = &state->game_draw;
    Model *card_model = find_model(&state->assets, "CARD");

    Ray mouse_ray;
    set_ray_coords(&mouse_ray, state->camera, state->scene, app->input.mouse, app->window.dim);

    #if VULKAN
    mouse_ray_model_intersections(draw->highlight_hover, mouse_ray, draw, card_model, game->active_player);
    #elif OPENGL
    mouse_ray_model_intersections_cpu(draw->highlight_hover, mouse_ray, draw, card_model, game->active_player);
    #endif // VULKAN / OPENGL

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
do_controller_selected_update(bool8 selected[SELECTED_SIZE],  Controller *controller) {
    for (u32 i = 0; i < HAND_SIZE; i++) {
        if (on_up(controller->buttons[i])) {
            selected[i] = true;
        }
    }    

    if (on_up(controller->pile)) {
        selected[PICKUP_PILE] = true;
    } else if (on_up(controller->discard)) {
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

internal void
print_vector3(const char *name, Vector3 v) {
    print("%s: %f, %f, %f\n", name, v.x, v.y, v.z);
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

bool8 update_game(State *state, App *app) {
    Game *game = &state->game;
    Game_Draw *draw = &state->game_draw;

    // Change camera mode or menu mode
    switch(state->camera_mode) {
        case FREE_CAMERA: {
            if (on_down(state->controller.pause)) {
                state->menu_list.mode = PAUSE_MENU;
                app->input.relative_mouse_mode = false;
                state->paused_earlier_in_frame = true;
            }
    
            if (on_down(state->controller.camera_toggle)) {
                state->camera_mode = PLAYER_CAMERA;
                app->input.relative_mouse_mode = false;
            }
        } break;

        case PLAYER_CAMERA: {
            if (on_up(state->controller.pause)) {
                state->menu_list.mode = PAUSE_MENU;
                state->paused_earlier_in_frame = true;                
            }
            
            if (on_down(state->controller.camera_toggle)) {
                state->camera_mode = FREE_CAMERA;
                app->input.relative_mouse_mode = true;
            }
        } break;
    }

    if (on_up(state->controller.save_camera)) {
        print_vector3("position", state->camera.position);
        print_vector3("target", state->camera.target);
        print_vector3("up", state->camera.up);
        print("%f\n", state->camera.fov);
        print("%f\n", state->camera.yaw);
        print("%f\n", state->camera.pitch);
    }
    
    // Game update
    switch(state->camera_mode) {
        case PLAYER_CAMERA: {
            // Game input
            if (game->round_type == HOLE_OVER) {
                float32 rot_speed = 100.0f * (float32)app->time.frame_time_s;
                float64 mouse_rot_speed = 0.2f;

                if (is_down(state->controller.right)) {
                    draw->rotation += rot_speed;
                }

                if (is_down(state->controller.left)) {
                    draw->rotation -= rot_speed;
                }

                if (on_down(state->controller.mouse_left)) {
                    draw->mouse_down = app->input.mouse;
                }

                if (is_down(state->controller.mouse_left)) {
                    float64 x_delta = float64(app->input.mouse.x - draw->mouse_down.x);
                    draw->rotation -= float32(x_delta * mouse_rot_speed);
                    draw->mouse_down = app->input.mouse;
                }
                
                break; // don't need to check game input or no game logic if hole over
            } 
    
            bool8 selected[SELECTED_SIZE] = {};
            
            if (state->is_active) {
                //if (app->input.active == MOUSE_INPUT)
                    do_mouse_selected_update(state, app, selected);
                //else if (app->input.active == KEYBOARD_INPUT)
                    do_controller_selected_update(selected, &state->controller);

                if (state->pass_selected) {
                    selected[PASS_BUTTON] = true;
                    state->pass_selected = false;
                }

                if (state->mode == MODE_CLIENT && !all_false(selected)) {
                    client_set_selected(online.sock, selected, state->client_game_index);
                }
            } else if (state->mode == MODE_SERVER && !game->players[game->active_player].is_bot) {
                os_wait_mutex(state->selected_mutex);
                if (!all_false(state->selected)) {
                    platform_memory_copy(selected, state->selected, sizeof(selected[0]) * SELECTED_SIZE);
                    platform_memory_set(state->selected, 0, sizeof(selected[0]) * SELECTED_SIZE);
                }
                os_release_mutex(state->selected_mutex);
            } else if (game->players[game->active_player].is_bot) {
                do_bot_selected_update(selected, game, &game->bot_thinking_time, app->time.frame_time_s);
            }

            // Timer
            if (state->mode != MODE_CLIENT) {
                float32 *time = &game->turn_time;
                *time += (float32)app->time.frame_time_s;
                
                if (*time > 10.0f) {
                    *time = 0.0f;
                    do_auto_selected_update(selected, game);
                };
            }
            
            if (state->mode == MODE_CLIENT || all_false(selected))
                 break; // client doesn't do any game updates       

            game->turn_time = 0.0f; // input received

            // Game logic
            do_update_with_input(game, selected);

            if (state->mode == MODE_SERVER) {
                server_send_game(game, draw_signals, DRAW_SIGNALS_AMOUNT);
            }
        } break;
    }

    // update card models
    do_draw_signals(draw_signals, game, draw);
    float32 rotation_degrees = process_rotation(&draw->camera_rotation, (float32)app->time.frame_time_s);
    
    // Update camera and card models after what happened in game update
    load_pile_card_models(game, draw, rotation_degrees, app->time.frame_time_s);
    do_card_animations(draw, (float32)app->time.frame_time_s);

    // Update Camera (uses rotation_degrees)
    switch(state->camera_mode) {
        case FREE_CAMERA: {
            if (app->input.relative_mouse_mode) {
                float64 mouse_m_per_s = 50.0;
                //float64 mouse_move_speed = mouse_m_per_s * app->time.frame_time_s;
                float64 mouse_move_speed = 0.1f;
                //print("%d %d == %d %d\n", mouse.x, mouse.y, app->input.mouse_rel.x, app->input.mouse_rel.y);
       
                update_camera_with_mouse(&state->camera, app->input.mouse_rel, mouse_move_speed, mouse_move_speed);
                update_camera_target(&state->camera); // I want target to be updated before I do this because it uses it
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
            float32 deg = 0.0f;

            if (game->round_type == HOLE_OVER) {
                deg = draw->rotation; // draw->rotation is set above
            } else if (state->mode == MODE_LOCAL) {
                draw->rotation = -draw->info.degrees_between_players * game->active_player;

                deg = -draw->info.degrees_between_players * game->active_player;
                deg += rotation_degrees;
            } else {
                draw->rotation = -draw->info.degrees_between_players * state->client_game_index;

                deg = -draw->info.degrees_between_players * state->client_game_index;
            }
            
            float32 rad = deg * DEG2RAD;
            float32 cam_dis = 11.5f + draw->info.radius;
            float32 x = cam_dis * cosf(rad);
            float32 y = cam_dis * sinf(rad);

            state->camera.position =  Vector3{ x, 12.0f, y };
            state->camera.yaw      = deg + 180.0f;
            state->camera.pitch    = -39.2f;

            update_camera_target(&state->camera);    
        } break;
    }

    state->scene.view = get_view(state->camera);
    
    return 0;
}

internal void
prepare_controller_for_input(Controller *controller) {
    for (u32 j = 0; j < ARRAY_COUNT(controller->buttons); j++) {
        controller->buttons[j].previous_state = controller->buttons[j].current_state;
    }
}

internal void
controller_process_input(Controller *controller, s32 id, bool8 state) {
    for (u32 i = 0; i < ARRAY_COUNT(controller->buttons); i++) {
        // loop through all ids associated with button
        for (u32 j = 0; j < controller->buttons[i].num_of_ids; j++) {
            if (id == controller->buttons[i].ids[j]) 
                controller->buttons[i].current_state = state;
        }
    }
}

bool8 update(App *app) {
    State *state = (State *)app->data;
    Assets *assets = &state->assets;

    bool8 full_menu = state->mode != MODE_CLIENT;
    state->is_active = (state->client_game_index == state->game.active_player || state->mode == MODE_LOCAL) && !state->game.players[state->game.active_player].is_bot;
    state->paused_earlier_in_frame = false;
    
    // Update
    if (state->menu_list.mode == IN_GAME) {
        if (state->mode == MODE_CLIENT) {
            os_wait_mutex(state->mutex);
        }
        update_game(state, app);
                    
        if (state->mode == MODE_CLIENT)
            os_release_mutex(state->mutex);
    } else {
        state->game_draw.name_plates_loaded = false;
    }

    // Draw
    os_wait_mutex(state->mutex);
    Shader *basic_3D = find_shader(assets, "BASIC3D");
    Shader *color_3D = find_shader(assets, "COLOR3D");

    for (u32 i = 0; i < 10; i++) {
        layouts[i].reset();
    }

    if (no_music_playing(&app->player))
        play_music("MORNING");
    
    mix_audio(&app->player, (float32)app->time.frame_time_s);
    queue_audio(&app->player);
    
    if (render_start_frame())
        goto AFTER_DRAW;

    texture_desc = render_get_descriptor_set_index(&layouts[2], 0);

    render_set_viewport(render_context.resolution.width, render_context.resolution.height);
    render_context.scissor_stack_index = 0;
    render_context.scissor_push({ 0, 0 }, cv2(app->window.dim));

    state->scene_set = render_get_descriptor_set(&layouts[0]);
    render_update_ubo(state->scene_set, (void*)&state->scene);

    state->scene_ortho_set = render_get_descriptor_set(&layouts[0]);
    render_update_ubo(state->scene_ortho_set, (void*)&state->ortho_scene);
    
    render_depth_test(false);

    light_set = render_get_descriptor_set(&layouts[1]);
    render_update_ubo(light_set, (void*)&global_light);

    light_set_2 = render_get_descriptor_set(&layouts[1]);
    render_update_ubo(light_set_2, (void*)&global_light_2);

    if (state->menu_list.mode != IN_GAME && state->menu_list.mode != PAUSE_MENU) {
        render_bind_pipeline(shapes.color_pipeline);
        render_bind_descriptor_set(state->scene_ortho_set);
    }

    switch(state->menu_list.mode) {
        case MAIN_MENU: {
            platform_memory_set(draw_signals, 0, sizeof(Draw_Signal) * DRAW_SIGNALS_AMOUNT);
            if (draw_main_menu(state, &state->menu_list.menus[MAIN_MENU], app->window.dim))
                return 1;
        } break;

        case LOCAL_MENU: draw_local_menu(state, &state->menu_list.menus[LOCAL_MENU], full_menu, app->window.dim); break;     
        case SCOREBOARD_MENU: draw_scoreboard(&state->menu_list.menus[SCOREBOARD_MENU], state, full_menu, app->window.dim); break;
        case HOST_MENU: draw_host_menu(&state->menu_list.menus[HOST_MENU], state, app->window.dim); break;
        case JOIN_MENU: draw_join_menu(&state->menu_list.menus[JOIN_MENU], state, app->window.dim); break;
        case SETTINGS_MENU: draw_settings_menu(&state->menu_list.menus[SETTINGS_MENU], state, app->window.dim); break;
        case VIDEO_SETTINGS_MENU: draw_video_settings_menu(&state->menu_list.menus[VIDEO_SETTINGS_MENU], state, &app->window); break;
        case AUDIO_SETTINGS_MENU: draw_audio_settings_menu(&state->menu_list.menus[AUDIO_SETTINGS_MENU], state, &app->player, &app->window); break;
        
        case PAUSE_MENU:
        case IN_GAME: {
                
            draw_game(state, &state->assets, basic_3D, &state->game, state->indices);

            // depth test already off from draw_game()
            render_bind_descriptor_set(state->scene_ortho_set);

            Font *font = find_font(&state->assets, "CASLON");
            float32 hole_pixel_height = app->window.dim.x / 30.0f;
            float32 pixel_height = app->window.dim.x / 20.0f;
            float32 padding = 10.0f;

            char hole_text[8];
            u32 hole_number = state->game.holes_played + 1;
            if (state->game.round_type == HOLE_OVER)
                hole_number -= 1;
            get_hole_text(hole_text, hole_number);

            String_Draw_Info hole_string_info = get_string_draw_info(font, hole_text, -1, hole_pixel_height);
            String_Draw_Info string_info = get_string_draw_info(font, round_types[state->game.round_type], -1, pixel_height);

            Vector2 hole_coords = { padding, padding };
            Vector2 round_coords = hole_coords + Vector2{ 0.0f, 2.0f + hole_string_info.dim.y };

            //draw_rect(hole_coords, 0, hole_string_info.dim, { 0, 0, 0, 0.5f} );            
            //draw_rect(round_coords - Vector2{ 5.0f, 5.0f }, 0, string_info.dim + Vector2{ 5.0f, 5.0f } * 2.0f, { 0, 0, 0, 0.8f} );      
            draw_string_tl(font, hole_text, hole_coords, hole_pixel_height, { 255, 255, 255, 1 });
            draw_string_tl(font, round_types[state->game.round_type], round_coords, pixel_height, { 255, 255, 255, 1 });

            gui.start();
            gui.rect.coords = { 0, 0 };
            gui.rect.dim = cv2(app->window.dim);
            gui.input = {
                &app->input.active,
                
                &state->controller.select,
                &state->controller.left,
                &state->controller.up,
                &state->controller.right,
                &state->controller.down,
                
                &app->input.mouse,
                &state->controller.mouse_left
            };

            float32 button_width = app->window.dim.x / 7.0f;
            Player *active_player = &state->game.players[state->game.active_player];

            if (state->menu_list.mode == PAUSE_MENU) {

                draw_pause_menu(state, &state->menu_list.menus[PAUSE_MENU], full_menu, app->window.dim);

            } else if (state->game.round_type == HOLE_OVER) {

                if (state->mode != MODE_CLIENT) {
                    Vector2 dim = { button_width, pixel_height };
                    Vector2 coords = { gui.rect.dim.x - dim.x - padding, gui.rect.dim.y - dim.y - padding };
                    if (gui_button(&gui, default_style, "Proceed", coords, dim)) {
                        state->menu_list.mode = SCOREBOARD_MENU;
                        if (state->mode == MODE_SERVER)
                            server_send_menu_mode(state->menu_list.mode);
                    }
                }

            } else if (state->game.turn_stage == FLIP_CARD && get_number_flipped(active_player->flipped) == HAND_SIZE - 1) {
                
                if (state->is_active) {
                    Vector2 dim = { button_width, pixel_height };
                    Vector2 coords = { app->window.dim.x - dim.x - padding, app->window.dim.y - dim.y - padding };
                    if (gui_button(&gui, default_style, "Pass", coords, dim)) {
                        state->pass_selected = true; // in update_game feed this into selected
                    }
                }

            }

        } break;
    }

    draw_onscreen_notifications(&state->notifications, app->window.dim, (float32)app->time.frame_time_s);

    draw_timer(state->game.turn_time, app->window.dim);
    

#ifdef DEBUG
    // Draw FPS
    char buffer[20];
    float_to_char_array((float32)app->time.frames_per_s, buffer, 20);
    draw_string_tl(default_font, buffer, { 10, (float32)app->window.dim.height - 40 }, 40.0f, { 255, 50, 50, 1 });
#endif // DEBUG

    render_end_frame(&state->assets, &app->window);
    AFTER_DRAW:

    os_release_mutex(state->mutex);
    prepare_controller_for_input(&state->controller);
    
    return 0;
}

internal void
update_scenes(Scene *scene, Scene *ortho_scene, Vector2_s32 window_dim) {
    //state->scene.view = look_at({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
    scene->projection = perspective_projection(45.0f, (float32)window_dim.width / (float32)window_dim.height, 0.1f, 1000.0f);

    ortho_scene->view = identity_m4x4();
    ortho_scene->projection = orthographic_projection(0.0f, (float32)window_dim.width, 0.0f, (float32)window_dim.height, -3.0f, 3.0f);
}

/*
https://www.freepik.com/free-vector/simple-realistic-wood-texture_1008177.htm#query=cartoon%20wood%20texture&position=3&from_view=keyword&track=ais&uuid=3c2d0918-a699-4f9b-b835-791d1dd2e14f
*/

internal void
convert_to_one_channel(Bitmap *bitmap) {
    u8 *new_memory = (u8*)platform_malloc(bitmap->width * bitmap->height * 1);

    for (s32 i = 0; i < bitmap->width * bitmap->height; i++) {
        u8 *src_ptr = bitmap->memory + (i * 4);
        u8 *dest_ptr = new_memory + i;
        *dest_ptr = src_ptr[3];
    }
    
    platform_free(bitmap->memory);
    bitmap->memory = new_memory;
    bitmap->channels = 1;
    bitmap->pitch = bitmap->width * bitmap->channels;
};

bool8 init_pipelines(Assets *assets) {
    Render_Pipeline *pipeline;
        
    pipeline = &pipelines[PIPELINE_2D_TEXT];
    pipeline->shader = find_shader(assets, "TEXT");
    pipeline->depth_test = false;
    init_basic_vert_layout(&pipeline->shader->set, layouts);
    init_text_frag_layout(pipeline->shader, layouts);
    render_create_graphics_pipeline(pipeline, get_vertex_xu_info());

    pipeline = &pipelines[PIPELINE_2D_COLOR];
    pipeline->shader = find_shader(assets, "COLOR");
    pipeline->depth_test = false;
    init_basic_vert_layout(&pipeline->shader->set, layouts);
    init_color_frag_layout(pipeline->shader, layouts);
    render_create_graphics_pipeline(pipeline, get_vertex_xu_info());
    
    pipeline = &pipelines[PIPELINE_2D_TEXTURE];
    pipeline->shader = find_shader(assets, "TEXTURE");
    pipeline->depth_test = false;
    init_basic_vert_layout(&pipeline->shader->set, layouts);
    init_texture_frag_layout(pipeline->shader, layouts);
    render_create_graphics_pipeline(pipeline, get_vertex_xu_info());

    pipeline = &pipelines[PIPELINE_3D_TEXTURE];
    pipeline->shader = find_shader(assets, "BASIC3D");
    pipeline->depth_test = true;
    init_basic_vert_layout(&pipeline->shader->set, layouts);
    init_basic_frag_layout(pipeline->shader, layouts);
    render_create_graphics_pipeline(pipeline, get_vertex_xnu_info());
    
    pipeline = &pipelines[PIPELINE_3D_COLOR];
    pipeline->shader = find_shader(assets, "COLOR3D");
    pipeline->depth_test = true;
    init_basic_vert_layout(&pipeline->shader->set, layouts);
    init_color3D_frag_layout(pipeline->shader, layouts);
    render_create_graphics_pipeline(pipeline, get_vertex_xnu_info());

    pipeline = &pipelines[PIPELINE_3D_TEXT];
    pipeline->shader = find_shader(assets, "TEXT3D");
    pipeline->depth_test = true;
    init_basic_vert_layout(&pipeline->shader->set, layouts);
    init_text_frag_layout(pipeline->shader, layouts);
    render_create_graphics_pipeline(pipeline, get_vertex_xnu_info());

    pipeline = &pipelines[PIPELINE_RAY];
    pipeline->shader = find_shader(assets, "RAY");
    pipeline->compute = true;
    init_ray_comp_layout(&pipeline->shader->set, layouts);
    render_create_compute_pipeline(pipeline);
    
    return false;
}

bool8 init_data(App *app) {
    app->data = platform_malloc(sizeof(State));
    State *state = (State *)app->data;
    *state = {};
    state->assets = {};

    global_assets = &state->assets;

    bool8 load_and_save_assets = false;
    
    if (load_and_save_assets) {
        if (load_asset_files_from_ethan(&state->assets, "../assets.ethan"))
            return true;
        save_asset_files(&state->assets, "files.save");
        if (load_assets(&state->assets))
            return true;

        convert_to_one_channel(find_bitmap(&state->assets, "BOT"));
    } else {
        const char *filepath = "assets.save";
        u32 offset = 0;

        FILE *file = fopen(filepath, "rb");
        if (file == 0) {
            load_saved_asset_files(&state->assets, "files.save");
            load_assets(&state->assets);
            load_and_save_assets = true;
            convert_to_one_channel(find_bitmap(&state->assets, "BOT"));
        } else {

            // have to compile then run the asset builder application to put the assets in the exe
            // const char *filepath = "river.exe";
            // u32 offset = exe_offset;
            fclose(file);
            if (load_saved_assets(&state->assets, filepath, offset))
            return 1;
        }
    }

    init_assets(&state->assets);

    default_font = find_font(&state->assets, "CASLON");

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
            const char *tag = "card";
            platform_memory_copy((void*)asset->tag, (void*)tag, 4);

            //write_bitmap(&asset->bitmap, &asset->file);
        };
        add_assets(&state->assets, card_assets, 14);
        print_assets(&state->assets);

        FILE *file = fopen("assets.save", "wb");
        save_assets(&state->assets, file);
        fclose(file);
    } else {
        for (u32 i = 0; i < 14; i++) {
            card_bitmaps[i] = state->assets.types[0].data[i + 8].bitmap; // + 8 to go after bitmaps loaded before the card bitmaps
        }
    }

    default_font = find_font(&state->assets, "CASLON");

    clear_font_bitmap_cache(default_font);
    
    global_assets = &state->assets;
    init_layouts(layouts, find_bitmap(&state->assets, "BACK"));
    
#if DEBUG
#if VULKAN
    vulkan_print_allocated_descriptors();
#endif // VULKAN
#endif // DEBUG

    init_pipelines(&state->assets);

    init_shapes(&state->assets);
    //clean_assets(&state->assets);

    // Rendering
    state->camera.position = { 0, 14, -5 };
    state->camera.target   = { 0, 0, 0 };
    state->camera.up       = { 0, -1, 0 };
    state->camera.fov      = 75.0f;
    state->camera.yaw      = 180.0f;
    state->camera.pitch    = -41.0f;
    state->camera_mode = PLAYER_CAMERA;

    update_scenes(&state->scene, &state->ortho_scene, app->window.dim);

    // Input
    set(&state->controller.forward, 'w');
    set(&state->controller.forward, SDLK_UP);
    set(&state->controller.backward, SDLK_s);
    set(&state->controller.backward, SDLK_DOWN);
    set(&state->controller.left, SDLK_a);
    set(&state->controller.left, SDLK_LEFT);
    set(&state->controller.right, SDLK_d);
    set(&state->controller.right, SDLK_RIGHT);

    set(&state->controller.up, SDLK_SPACE);
    set(&state->controller.down, SDLK_LSHIFT);

    set(&state->controller.select, SDLK_RETURN);
    set(&state->controller.pause,  SDLK_ESCAPE);
    set(&state->controller.pass,   SDLK_p); 

#ifdef DEBUG
    set(&state->controller.camera_toggle, SDLK_c);
    set(&state->controller.save_camera, SDLK_v);
#endif // DEBUG

    set(&state->controller.one,   SDLK_u);
    set(&state->controller.two,   SDLK_i);
    set(&state->controller.three, SDLK_o);
    set(&state->controller.four,  SDLK_p);

    set(&state->controller.five,  SDLK_j);
    set(&state->controller.six,   SDLK_k);
    set(&state->controller.seven, SDLK_l);
    set(&state->controller.eight, SDLK_SEMICOLON);

    set(&state->controller.pile,   SDLK_9);
    set(&state->controller.discard,SDLK_0);
    
    set(&state->controller.one,   SDLK_q);
    set(&state->controller.two,   SDLK_w);
    set(&state->controller.three, SDLK_e);
    set(&state->controller.four,  SDLK_r);

    set(&state->controller.five,  SDLK_a);
    set(&state->controller.six,   SDLK_s);
    set(&state->controller.seven, SDLK_d);
    set(&state->controller.eight, SDLK_f);

    set(&state->controller.pile,    SDLK_1);
    set(&state->controller.discard, SDLK_2);

    set(&state->controller.mouse_left, SDL_BUTTON_LEFT);

    // Setting default Menus
    Menu default_menu = {};
    default_menu.gui.style.background_color = play_nine_yellow;
    default_menu.gui.style.background_color_hover = play_nine_light_yellow;
    default_menu.gui.style.background_color_pressed = play_nine_dark_yellow;
    default_menu.gui.style.background_color_active = play_nine_yellow;
    
    default_menu.gui.style.text_color = play_nine_green;
    default_menu.gui.style.text_color_hover = play_nine_green;
    default_menu.gui.style.text_color_pressed = play_nine_green;
    default_menu.gui.style.text_color_active = play_nine_green;
    
    default_menu.gui.edit.index = 0;
    
    default_menu.gui.font = default_font;
    default_menu.gui.input = {
        &app->input.active,
        
        &state->controller.select,
        &state->controller.left,
        &state->controller.forward,
        &state->controller.right,
        &state->controller.backward,
        
        &app->input.mouse,
        &state->controller.mouse_left,

        app->input.buffer,
        &app->input.buffer_index
    };

    default_style = default_menu.gui.style;

    for (u32 i = 0; i < IN_GAME; i++) {
        state->menu_list.menus[i] = default_menu;
    }

    gui.font = default_font;

    // Online
    state->mutex = os_create_mutex();
    state->selected_mutex = os_create_mutex();

    Descriptor texture_desc = render_get_descriptor_set_index(&layouts[2], 0);

    for (u32 j = 0; j < 14; j++) {
        state->indices[j] = render_set_bitmap(&texture_desc, &card_bitmaps[j]);
    }
    state->indices[14] = render_set_bitmap(&texture_desc, find_bitmap(&state->assets, "BACK"));
    Model *model = find_model(&state->assets, "TABLE");
    state->indices[15] = render_set_bitmap(&texture_desc, &model->meshes[0].material.diffuse_map);
    
    // General Game

    init_triangles(find_model(&state->assets, "CARD")); // Fills array on graphics card
    init_deck();

    state->game_draw.bot_bitmap = find_bitmap(&state->assets, "BOT");

    state->notifications.font = default_font;
    state->notifications.text_color = { 255, 255, 255, 1 };

    global_mode = &state->mode;

    return false;
}

s32 event_handler(App *app, App_System_Event event, u32 arg) {
    State *state = (State *)app->data;

    switch(event) {
        case APP_INIT: {
            render_clear_color(Vector4{ 0.0f, 0.0f, 0.0f, 1.0f });

            // draw loading screen
            //render_start_frame();
            //render_end_frame();

            init_audio_player(&app->player);

            app->update = &update;
            if (init_data(app))
                return 1;
            State *state = (State *)app->data;
            app->icon = find_bitmap(&state->assets, "ICON");
        } break;

        case APP_EXIT: {
            print("APP_EXIT\n");
            if (state->mode == MODE_CLIENT) {
                client_close_connection(online.sock);
            } else if (state->mode == MODE_SERVER) {
                close_server();
            }

            render_assets_cleanup(&state->assets);

            for (u32 i = 0; i < PIPELINE_COUNT; i++) {
                render_pipeline_cleanup(&pipelines[i]);
            }

            unload_name_plates(&state->game_draw);
        } break;

        case APP_RESIZED: {
            update_scenes(&state->scene, &state->ortho_scene, app->window.dim);
        }

        case APP_KEYDOWN: {
            u32 previous_active = app->input.active;
            app->input.active = KEYBOARD_INPUT;
            if (previous_active == MOUSE_INPUT && state->menu_list.mode != IN_GAME && arg != 27) break;
            controller_process_input(&state->controller, arg, true);
        } break;

        case APP_KEYUP: {
            u32 previous_active = app->input.active;
            app->input.active = KEYBOARD_INPUT;
            if (previous_active == MOUSE_INPUT && state->menu_list.mode != IN_GAME && arg != 27) break;
            controller_process_input(&state->controller, arg, false);
        } break;

        case APP_MOUSEDOWN: {
            app->input.active = MOUSE_INPUT;
            controller_process_input(&state->controller, arg, true);
        } break;

        case APP_MOUSEUP: {
            app->input.active = MOUSE_INPUT;
            controller_process_input(&state->controller, arg, false);
        } break;
    }

    return 0;
}

