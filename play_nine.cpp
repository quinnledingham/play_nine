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

    render_draw_model(find_model(assets, "CARD"), shader, position, get_rotation(degrees * DEG2RAD, {0, -1, 0}), &card_bitmaps[bitmap_index]);
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

//      180
// 270        90
//       0
internal void
draw_game(Assets *assets, Shader *shader, Game *game) {
    float32 hyp = 7.0f;
    float32 deg = 360.0f / float32(game->num_of_players);
    float32 rad = deg * DEG2RAD;
    for (u32 i = 0; i < game->num_of_players; i++) {
        Vector3 position = { 0, 0, 0 };
        position.x = hyp * cosf(i * rad);
        position.z = hyp * sinf(i * rad);
        draw_player_cards(assets, shader, &game->players[i], position, (i * deg) + 90.0f);
    }
}

bool8 init_data(App *app) {
	app->data = platform_malloc(sizeof(State));
	platform_memory_set(app->data, 0, sizeof(State));
    State *game = (State *)app->data;
	game->assets = {};
	load_assets(&game->assets, "../assets.ethan");

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
    game->camera.yaw      = 0.0f;
    game->camera.pitch    = 0.0f;
	
	game->rect = get_rect_mesh();
	
	set(&game->controller.forward, 'w');
	set(&game->controller.backward, SDLK_s);
	set(&game->controller.left, SDLK_a);
	set(&game->controller.right, SDLK_d);
	set(&game->controller.up, SDLK_SPACE);
	set(&game->controller.down, SDLK_LSHIFT);

	set(&game->controller.pause, SDLK_ESCAPE);

    init_deck();
    init_card_bitmaps(card_bitmaps, font);
    shuffle_pile(game->game.pile);
    game->game.num_of_players = 3;
    deal_cards(&game->game);

    game->test = create_string_into_bitmap(font, 500.0f, "yo");
    render_create_texture(&game->test, TEXTURE_PARAMETERS_CHAR);
    clear_font_bitmap_cache(font);
    
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
	State *game = (State *)app->data;
	Assets *assets = &game->assets;

    if (app->window.resized) {
        game->scene.projection = perspective_projection(45.0f, (float32)app->window.width / (float32)app->window.height, 0.1f, 1000.0f);
        render_update_ubo(game->scene_set, 0, (void*)&game->scene, true);

        game->ortho_scene.projection = orthographic_projection(0.0f, (float32)app->window.width, 0.0f, (float32)app->window.height, -3.0f, 3.0f);
        render_update_ubo(game->scene_ortho_set, 0, (void*)&game->ortho_scene, true);
    }

	if (on_down(game->controller.pause)) {
		app->input.relative_mouse_mode = !app->input.relative_mouse_mode;
        game->game.top_of_pile = 0;
        shuffle_pile(game->game.pile);
        deal_cards(&game->game);
	}

	if (app->input.relative_mouse_mode) {
		float32 mouse_m_per_s = 100.0f;
		float32 mouse_move_speed = mouse_m_per_s * app->time.frame_time_s;
		update_camera_with_mouse(&game->camera, app->input.mouse_rel, { mouse_move_speed, mouse_move_speed });
		update_camera_target(&game->camera);	
	    game->scene.view = get_view(game->camera);
		render_update_ubo(game->scene_set, 0, (void*)&game->scene, true);

		float32 m_per_s = 6.0f; 
        float32 m_moved = m_per_s * app->time.frame_time_s;
        Vector3 move_vector = {m_moved, m_moved, m_moved};
        update_camera_with_keys(&game->camera, game->camera.target, game->camera.up, move_vector,
                                is_down(game->controller.forward),  is_down(game->controller.backward),
                                is_down(game->controller.left),  is_down(game->controller.right),
                                is_down(game->controller.up),  is_down(game->controller.down));								

		//print("%f %f %f\n", game->camera.position.x, game->camera.position.y, game->camera.position.z);
	}

    Shader *basic_3D = find_shader(assets, "BASIC3D");

    render_start_frame();

    render_set_viewport(app->window.width, app->window.height);
    render_set_scissor(app->window.width, app->window.height);

    render_bind_pipeline(&game->basic_pipeline);

    render_bind_descriptor_set(game->scene_set, 0);
    {
        Matrix_4x4 model = create_transform_m4x4({ 0.0f, 0.0f, 0.0f }, get_rotation(0, {0, 1, 0}), {1, 1, 1.0f});
        render_push_constants(&basic_3D->layout_sets[2], (void *)&model);

        Descriptor_Set *object_set = render_get_descriptor_set(basic_3D, 1);
        render_set_bitmap(object_set, &game->test, 1);
        render_bind_descriptor_set(object_set, 1);

        render_draw_mesh(&game->rect);

        //draw_player_cards(&game->assets, basic_3D, &game->game.players[0], { 0, 0, 0 }, 0);
        draw_game(&game->assets, basic_3D, &game->game);
    }

    render_bind_descriptor_set(game->scene_ortho_set, 0);
    {
        Matrix_4x4 model = create_transform_m4x4({ 100.0f, 100.0f, -0.5f }, get_rotation(0, {0, 1, 0}), {100, 100, 1.0f});
        render_push_constants(&basic_3D->layout_sets[2], (void *)&model);

        Descriptor_Set *object_set = render_get_descriptor_set(basic_3D, 1);
        render_set_bitmap(object_set, find_bitmap(&game->assets, "DAVID"), 1);
        render_bind_descriptor_set(object_set, 1);

        render_draw_mesh(&game->rect);
        
        draw_string(find_font(&game->assets, "CASLON"), "supgamer", {200, 200}, 100.0f, { 255, 0, 255, 1 });
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
            init_data(app);
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