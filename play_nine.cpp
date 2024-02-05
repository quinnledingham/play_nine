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
#include "play_nine.h"

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
        for (u32 card_index = 0; card_index < 8; card_index++) {
            game->players[i].cards[card_index] = game->pile[game->top_of_pile++];
        }
    }

    float32 card_width = 2.0f;
    float32 card_height = 3.2f;
    float32 padding = 0.2f;

    u32 card_index = 0;
    for (s32 y = 1; y >= 0; y--) {
        for (s32 x = -2; x <= 1; x++) {
             hand_coords[card_index++] = { 
                float32(x) * card_width + (float32(x) * padding) + (card_width / 2.0f) + (padding / 2.0f), 
                float32(y) * card_height + (float32(y) * padding) - (card_height / 2.0f) - (padding / 2.0f)
            };
        }
    }
}

internal void
copy_bitmap_into_bitmap(Bitmap dest, const Bitmap src, Vector2_s32 position) {
    u8 *dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch);
    u8 *src_ptr = src.memory;
    for (s32 y = 0; y < src.height; y++) {
        for (s32 x = 0; x < src.width; x++) {   
            u8 color = 0xFF ^ (*src_ptr);
            
            for (s32 i = 0; i < dest.channels; i++) {
                for (s32 j = 0; j < src.channels; j++)
                    dest_ptr[i] = 0xFF ^ (*src_ptr);
            }

            dest_ptr += dest.channels;
            src_ptr += src.channels;
        }   

        dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch) + (y * dest.pitch);
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
    memset(bitmap.memory, 0xFF, bitmap.width * bitmap.height * bitmap.channels);

    current_point = 0.0f;

    i = 0;
    while (str[i] != 0 ) {
        Font_Char_Bitmap *fbitmap = load_font_char_bitmap(font, str[i], scale);
        Font_Char *font_char = fbitmap->font_char;

        Vector2 char_coords = { current_point + (font_char->lsb * scale) - left, (float32)0 };

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

    u32 x_center = (bitmap.width / 2) - (str_bitmap.width / 2);
    u32 y_center = (bitmap.height / 2) - (str_bitmap.height / 2);
    copy_bitmap_into_bitmap(bitmap, str_bitmap, { s32(x_center), s32(y_center) });

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

internal void
draw_card(Assets *assets, Shader *shader, Card card, Vector3 position, float32 degrees) {
    u32 bitmap_index = 0;
    if (card.number == -5)
        bitmap_index = 13;
    else 
        bitmap_index = card.number;

    render_draw_model(find_model(assets, "CARD"), shader, position, get_rotation(degrees * DEG2RAD, {0, -1, 0}), &card_bitmaps[bitmap_index], find_bitmap(assets, "YOGI"));
}

internal void
draw_player_cards(Assets *assets, Shader *shader, Player *player, Vector3 position, float32 degrees) {
    float32 rad = -degrees * DEG2RAD;     

    for (u32 i = 0; i < 8; i++) {
        Vector3 card_pos = { hand_coords[i].x, 0.0f, hand_coords[i].y };
        card_pos = { 
            cosf(rad) * card_pos.x + sinf(rad) * card_pos.z, 
            0.0f, 
            -sinf(rad) * card_pos.x + cosf(rad) * card_pos.z 
        };
        card_pos += position;
        draw_card(assets, shader, deck[player->cards[i]], card_pos, degrees);
    }
}   

internal Vector3
get_hand_position(float32 hyp, float32 deg, u32 i) {
    float32 rad = deg * DEG2RAD;
    Vector3 position = { 0, 0, 0 };
    position.x = hyp * cosf(i * rad);
    position.z = hyp * sinf(i * rad);
    return position;
}

//      180
// 270        90
//       0
internal void
draw_game(Assets *assets, Shader *shader, Game *game) {
    for (u32 i = 0; i < game->num_of_players; i++) {
        Vector3 position = get_hand_position(game->radius, game->degrees_between_players, i);
        draw_player_cards(assets, shader, &game->players[i], position, (game->degrees_between_players * i) + 90.0f);
    }
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
    game->basic_pipeline.shader = basic_3D;

	render_create_graphics_pipeline(&game->basic_pipeline, get_vertex_xnu_info());
	
	// Init assets
	Bitmap *yogi = find_bitmap(&game->assets, "YOGI");
    render_create_texture(yogi, TEXTURE_PARAMETERS_DEFAULT);

	Bitmap *david = find_bitmap(&game->assets, "DAVID");
    render_create_texture(david, TEXTURE_PARAMETERS_DEFAULT);
	
	Font *font = find_font(&game->assets, "CASLON");
    init_font(font);

	render_init_model(find_model(&game->assets, "TAILS"));
    render_init_model(find_model(&game->assets, "CARD"));

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
	
	game->rect = get_rect_mesh();
	
	set(&game->controller.forward, 'w');
	set(&game->controller.backward, SDLK_s);
	set(&game->controller.left, SDLK_a);
	set(&game->controller.right, SDLK_d);
	set(&game->controller.up, SDLK_SPACE);
	set(&game->controller.down, SDLK_LSHIFT);

    set(&game->controller.select, SDLK_RETURN);
	set(&game->controller.pause,  SDLK_ESCAPE);

    init_deck();
    init_card_bitmaps(card_bitmaps, font);

    clear_font_bitmap_cache(font);
    
    init_shapes(&game->assets);

    game->camera_mode = PLAYER_CAMERA;

	return false;
}

internal void
start_game(Game *game, u32 num_of_players) {
    shuffle_pile(game->pile);
    game->num_of_players = num_of_players;
    deal_cards(game);

    game->degrees_between_players = 360.0f / float32(game->num_of_players);
    game->radius = 7.0f;
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

internal void
menu_update_active(s32 *active, s32 lower, s32 upper, Button increase, Button decrease) {
    if (on_down(increase)) {
        (*active)++;
        if (*active > upper)
            *active = upper;
    }
    if (on_down(decrease)) {
        (*active)--;
        if (*active < lower)
            *active = lower;
    }
}

// returns game mode
internal s32
draw_main_menu(State *game, Font *font, Controller *controller, Vector2_s32 window_dim)
{
    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);

    Menu main_menu = {};
    main_menu.font = font;
    main_menu.rect = get_centered_rect(window_rect, 0.5f, 0.5f);

    main_menu.button_style.default_back_color = {  34,  44, 107, 1 };
    main_menu.button_style.active_back_color  = {  42,  55, 131, 1 };
    main_menu.button_style.default_text_color = { 234,   0,  39, 1 };
    main_menu.button_style.active_text_color  = { 171, 160, 200, 1 };;
    
    u32 buttons_count = 3;
    main_menu.button_style.dim = { main_menu.rect.dim.x, main_menu.rect.dim.y / float32(buttons_count) };

    bool8 select = on_down(controller->select);
    u32 index = 0;

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 37, 38, 90, 1 } );
    draw_rect(main_menu.rect.coords, 0, main_menu.rect.dim, { 0, 0, 0, 0.2f} );

    if (menu_button(&main_menu, "Local",  index++, game->active, select)) {
        game->mode = LOCAL;
        start_game(&game->game, 4);
    }
    if (menu_button(&main_menu, "Online", index++, game->active, select))
        game->mode = ONLINE;    
    if (menu_button(&main_menu, "Quit",   index++, game->active, select)) 
        return true;

    return false;
}

bool8 update_game(State *state, App *app) {
    switch(state->camera_mode) {
        case FREE_CAMERA: {
            if (on_down(state->controller.pause)) {
/*
                app->input.relative_mouse_mode = !app->input.relative_mouse_mode;
                state->game.top_of_pile = 0;
                shuffle_pile(state->game.pile);
                deal_cards(&state->game);
*/
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

                print("%f %f %f\n", state->camera.position.x, state->camera.position.y, state->camera.position.z);
            }
        } break;

        case PLAYER_CAMERA: {
            if (on_down(state->controller.pause)) {
                state->camera_mode = FREE_CAMERA;
                app->input.relative_mouse_mode = true;
            }
            
            Game *game = &state->game;
            if (on_down(state->controller.right)) {
                game->active_player++;
                if (game->active_player >= game->num_of_players)
                    game->active_player = 0;
            }


            Vector3 position = get_hand_position(game->radius, game->degrees_between_players, game->active_player);
            float32 cam_dis = 8.0f;
            float32 deg = game->degrees_between_players * game->active_player;
            float32 rad = deg * DEG2RAD;
            float32 x = cam_dis * cosf(rad);
            float32 y = cam_dis * sinf(rad);

            state->camera.position = position + Vector3{ x, 14.0f, y };
            state->camera.yaw      = deg + 180.0f;
            state->camera.pitch    = -50.0f;

            update_camera_target(&state->camera);    
            state->scene.view = get_view(state->camera);
            render_update_ubo(state->scene_set, 0, (void*)&state->scene, true);
        } break;
    }

    return 0;
}

bool8 update(App *app) {
	State *game = (State *)app->data;
	Assets *assets = &game->assets;

    if (app->window.resized) {
        game->scene.projection = perspective_projection(45.0f, (float32)app->window.width / (float32)app->window.height, 0.1f, 1000.0f);
        render_update_ubo(game->scene_set, 0, (void*)&game->scene, true);

        game->ortho_scene.projection = orthographic_projection(0.0f, (float32)app->window.width, 0.0f, (float32)app->window.height, -3.0f, 3.0f);
        render_update_ubo(game->scene_ortho_set, 0, (void*)&game->ortho_scene, true);
    }
    
    // Update
    switch(game->mode) {
        case MAIN_MENU: {
            menu_update_active(&game->active, 0, 2, game->controller.down,  game->controller.up);
        } break;

        case LOCAL: {
            update_game(game, app);
        }
    }

    // Draw
    Shader *basic_3D = find_shader(assets, "BASIC3D");

    vulkan_reset_descriptor_sets(assets);
    render_start_frame();

    render_set_viewport(app->window.width, app->window.height);
    render_set_scissor(app->window.width, app->window.height);

    switch(game->mode) {
        case MAIN_MENU: {
            render_bind_pipeline(&shapes.color_pipeline);
            render_bind_descriptor_set(game->scene_ortho_set, 0);
            if (draw_main_menu(game, find_font(assets, "CASLON"), &game->controller, app->window.dim))
                return 1;
        } break;

        case LOCAL: {
            render_bind_pipeline(&game->basic_pipeline);
            render_bind_descriptor_set(game->scene_set, 0);



            draw_game(&game->assets, basic_3D, &game->game);
        } break;
    }


    render_end_frame();

    prepare_controller_for_input(&game->controller);

	return 0;
}

s32 event_handler(App *app, App_System_Event event, u32 arg) {
    State *game = (State *)app->data;

    switch(event) {
        case APP_INIT: {
            app->update = &update;
            if (init_data(app))
                return 1;
        } break;

        case APP_KEYDOWN: {
            controller_process_input(&game->controller, arg, true);
        } break;

        case APP_KEYUP: {
            controller_process_input(&game->controller, arg, false);
        } break;
    }

    return 0;
}