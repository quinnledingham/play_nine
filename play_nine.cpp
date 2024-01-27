#include <stdarg.h>
#include <cstdint>
#include <ctype.h>
#include <stdio.h>

#define internal      static
#define local_persist static
#define global        static

#include "types.h"

void *platform_malloc(u32 size);
void platform_free(void *ptr);
void platform_memory_set(void *dest, s32 value, u32 num_of_bytes);

#include "print.h"
#include "char_array.h"
#include "assets.h"

#include "application.h"
#include "play_nine.h"

bool8 init_data(App *app) {
	app->data = platform_malloc(sizeof(Game));
	platform_memory_set(app->data, 0, sizeof(Game));
    Game *game = (Game *)app->data;
	game->assets = {};
	load_assets(&game->assets, "../assets.ethan");

	Shader *basic_3D = find_shader(&game->assets, "BASIC3D");
    render_compile_shader(basic_3D);

    basic_3D->layout_sets[0].descriptors[0] = Descriptor(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, sizeof(Scene), descriptor_scope::GLOBAL);
    basic_3D->layout_sets[0].descriptors_count = 1;
    render_create_descriptor_pool(basic_3D, 30, 0);

    basic_3D->layout_sets[1].descriptors[0] = Descriptor(1, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, 0, descriptor_scope::GLOBAL);
    basic_3D->layout_sets[1].descriptors_count = 1;
    render_create_descriptor_pool(basic_3D, 30, 1);

    basic_3D->layout_sets[2].descriptors[0] = Descriptor(SHADER_STAGE_VERTEX, sizeof(Matrix_4x4), descriptor_scope::LOCAL);
    basic_3D->layout_sets[2].descriptors_count = 1;

    game->basic_pipeline.shader = basic_3D;
	render_create_graphics_pipeline(&game->basic_pipeline);
	
	// Init assets
	Bitmap *yogi = find_bitmap(&game->assets, "YOGI");
    render_create_texture(yogi, TEXTURE_PARAMETERS_DEFAULT);

	Bitmap *david = find_bitmap(&game->assets, "DAVID");
    render_create_texture(david, TEXTURE_PARAMETERS_DEFAULT);
	
	Font *font = find_font(&game->assets, "CASLON");
    init_font(font);

	render_init_model(find_model(&game->assets, "TAILS"));

	// Rendering
	game->scene.view = look_at({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
    game->scene.projection = perspective_projection(45.0f, (float32)app->window.width / (float32)app->window.height, 0.1f, 1000.0f);

    // Init ubo
    game->scene_set = render_get_descriptor_set(basic_3D, 0);
    render_update_ubo(game->scene_set, 0, (void*)&game->scene, true);

    game->scene_ortho_set = render_get_descriptor_set(basic_3D, 0);
    game->ortho_scene.view = identity_m4x4();
    game->ortho_scene.projection = orthographic_projection(0.0f, (float32)app->window.width, 0.0f, (float32)app->window.height, -3.0f, 3.0f);
    render_update_ubo(game->scene_ortho_set, 0, (void*)&game->ortho_scene, true);

    game->camera.position = { 2, 2, 2 };
    game->camera.target   = { 0, 0, 0 };
    game->camera.up       = { 0, -1, 0 };
    game->camera.fov      = 75.0f;
    game->camera.yaw      = 0.0f;
    game->camera.pitch    = 0.0f;
	
	game->rect = get_rect_mesh();
	
	set(&game->controller.forward, SDLK_w);
	set(&game->controller.backward, SDLK_s);
	set(&game->controller.left, SDLK_a);
	set(&game->controller.right, SDLK_d);
	set(&game->controller.up, SDLK_SPACE);
	set(&game->controller.down, SDLK_LSHIFT);

	set(&game->controller.pause, SDLK_ESCAPE);

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
	Game *game = (Game *)app->data;
	Assets *assets = &game->assets;
	
	prepare_controller_for_input(&game->controller);
	for (u32 i = 0; i < app->input.key_events_count; i++) {
		controller_process_input(&game->controller, app->input.key_events[i].id, app->input.key_events[i].state);
	}

	if (on_down(game->controller.pause)) {
		app->input.relative_mouse_mode = !app->input.relative_mouse_mode;
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

		print("%f %f %f\n", game->camera.position.x, game->camera.position.y, game->camera.position.z);
	}

    render_start_frame();

    render_set_viewport(app->window.width, app->window.height);
    render_set_scissor(app->window.width, app->window.height);

    render_bind_pipeline(&game->basic_pipeline);
    Vector4 test = { 1, 1, 0, 1 };
    
	Shader *basic_3D = find_shader(assets, "BASIC3D");

    render_bind_descriptor_set(game->scene_set, 0);
    {            
        Matrix_4x4 model = create_transform_m4x4({ 0.0f, 0.0f, 0.0f }, get_rotation(0, {0, 1, 0}), {1.0f, 1.0f, 1.0f});
        render_push_constants(&basic_3D->layout_sets[2], (void *)&model);

        Descriptor_Set *object_set = render_get_descriptor_set(basic_3D, 1);
        render_set_bitmap(object_set, find_bitmap(&game->assets, "YOGI"), 1);
        render_bind_descriptor_set(object_set, 1);

        render_draw_mesh(&game->rect);

		render_draw_model(find_model(&game->assets, "TAILS"), basic_3D, { 0, 0, 0 }, get_rotation(0, {0, 1, 0}));
    }
    
    render_bind_descriptor_set(game->scene_ortho_set, 0);
    { 
        Matrix_4x4 model = create_transform_m4x4({ 100.0f, 100.0f, -0.5f }, get_rotation(0, {0, 1, 0}), {100, 100, 1.0f});
        render_push_constants(&basic_3D->layout_sets[2], (void *)&model);

        Descriptor_Set *object_set = render_get_descriptor_set(basic_3D, 1);
        render_set_bitmap(object_set, find_bitmap(&game->assets, "DAVID"), 1);
        render_bind_descriptor_set(object_set, 1);

        render_draw_mesh(&game->rect);
    }
    
    draw_string(find_font(&game->assets, "CASLON"), "supgamer", {200, 200}, 100.0f, { 255, 0, 255, 1 });

    render_end_frame();

	return 0;
}