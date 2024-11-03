#include "basic.cpp"

#include "play_nine_game.h"
#include "play_nine_assets.h"
#include "play_nine_raytrace.h"
#include "play_nine_shaders.h"
#include "play_nine_input.h"
#include "play_nine_draw.h"
#include "play_nine.h"
#include "play_nine_render.h"
#include "play_nine_online.h"

#include "play_nine_score.cpp"
#include "play_nine_init.cpp"
#include "play_nine_game.cpp"

#include "play_nine_raytrace.cpp"
#include "play_nine_online.cpp"
#include "play_nine_bitmaps.cpp"
#include "input.cpp"
#include "play_nine_menus.cpp"
#include "play_nine_draw.cpp"
#include "play_nine_bot.cpp"

bool8 update(App *app) {
    State *state = (State *)app->data;

    // Resets all of the descriptor sets to be reallocated on next frame
    for (u32 i = 0; i < GFX_ID_COUNT; i++) {
        gfx.layouts[i].reset();
    }

    state->scene_ortho_set = gfx.descriptor_set(GFX_ID_SCENE);
    gfx.update_ubo(state->scene_ortho_set, (void*)&state->ortho_scene);

   if (gfx.start_frame())
        return 0;

    gfx.depth_test(false);

    gfx.bind_shader("COLOR");
    gfx.bind_descriptor_set(state->scene_ortho_set);

    texture_atlas_draw(input_prompt_atlases[0], 0, {100, 100}, {300, 300});

    gfx.end_frame(gfx.get_flags());

    return 0;
}

internal void
update_scenes(Scene *scene, Scene *ortho_scene, Vector2_s32 window_dim) {
    //state->scene.view = look_at({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
    scene->projection = perspective_projection(45.0f, (float32)window_dim.width / (float32)window_dim.height, 0.1f, 1000.0f);

    ortho_scene->view = identity_m4x4();
    ortho_scene->projection = orthographic_projection(0.0f, (float32)window_dim.width, 0.0f, (float32)window_dim.height, -3.0f, 3.0f);
}

bool8 init_data(App *app) {
    app->data = platform_malloc(sizeof(State));
    State *state = (State *)app->data;
    *state = {};
    state->assets = {};

    Bitmap blank_layout_bitmap = blank_bitmap(32, 32, 4);
    gfx.create_texture(&blank_layout_bitmap, TEXTURE_PARAMETERS_DEFAULT);
    gfx.layouts = ARRAY_MALLOC_CLEAR(Layout, 11);
    init_layouts(gfx.layouts, &blank_layout_bitmap);

    bool8 reload_bot_bitmap = true;
    if (reload_bot_bitmap) {
        File bot_file = load_file("../assets/bitmaps/bot_original.png");
        Bitmap bot_bitmap = load_bitmap(bot_file, false); 
        bitmap_convert_channels(&bot_bitmap, 1);
        write_bitmap(&bot_bitmap, "../assets/bitmaps/bot.png");
    }

    if (load_assets(&state->assets, assets_to_load, ARRAY_COUNT(assets_to_load), false)) {
        logprint("init_data()", "load_assets failed\n");
        return 1;
    }

    global_assets = &state->assets;

    init_pipelines(&state->assets);
    gfx.init_shapes();
    update_scenes(&state->scene, &state->ortho_scene, app->window.dim);

    init_triangles(find_model(&state->assets, "CARD"), &state->triangle_desc); // Fills array on graphics card
    init_deck();

    input_prompt_atlases[PROMPT_KEYBOARD] = find_texture_atlas(&state->assets, "KEYBOARD_PROMPT");
    input_prompt_atlases[PROMPT_XBOX_SERIES] = find_texture_atlas(&state->assets, "XBOX_SERIES_PROMPT");

    return 0;
}

s32 event_handler(App *app, App_System_Event event, u32 arg) {
    switch(event) {
        case APP_INIT: {
            gfx.clear_color(Vector4{ 255.0f, 255.0f, 255.0f, 1.0f });
            app->update = &update;

            gfx.start_frame();
            gfx.end_frame(gfx.get_flags());

            init_data(app);
        } break;
    }
    
    return 0;
}
