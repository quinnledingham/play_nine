#include "basic.cpp"

bool8 update(App *app) {
    if (app->window.resized) {
        gfx.create_swap_chain();
        app->window.resized = false;
    }

   if (gfx.start_frame())
        return 0;

    gfx.end_frame(gfx.get_flags());

    return 0;
}

s32 event_handler(App *app, App_System_Event event, u32 arg) {
    switch(event) {
        case APP_INIT: {
            gfx.clear_color(Vector4{ 255.0f, 50.0f, 0.0f, 1.0f });
            app->update = &update;
        } break;
    }
    
    return 0;
}
