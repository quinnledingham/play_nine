internal void
sdl_update_time(App_Time *time) {
    s64 ticks = SDL_GetPerformanceCounter();

    // s
    time->frame_time_s = get_seconds_elapsed(time, time->last_frame_ticks, ticks);
    time->last_frame_ticks = ticks; // set last ticks for next frame

    // time->start has to be initialized before
    time->run_time_s = get_seconds_elapsed(time, time->start_ticks, ticks);

    // fps
    time->frames_per_s = 1.0 / time->frame_time_s;
}

internal void
sdl_set_fullscreen(SDL_Window *sdl_window, enum Display_Modes display_mode) {
    switch(display_mode) {
        case DISPLAY_MODE_WINDOWED: 
            if (SDL_SetWindowFullscreen(sdl_window, 0) < 0)
                print(SDL_GetError());      
        break;
        //case DISPLAY_MODE_FULLSCREEN: SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN); break;
        case DISPLAY_MODE_WINDOWED_FULLSCREEN: SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP); break;
    }
}

internal void
sdl_get_clipboard_text(App_Input *input) {
    char *clipboard_text = SDL_GetClipboardText();
    
    if (!clipboard_text) {
        print(SDL_GetError());
        return;
    }
    
    app_copy_string_to_input_buffer(input, clipboard_text);
}

struct SDL_Input {
    SDL_Joystick *joysticks[4];
    SDL_GameController *game_controllers[4];
    u32 num_of_joysticks;
    u32 num_of_game_controllers;

};

internal void
sdl_init_controllers(SDL_Input *input) {
    input->num_of_joysticks = SDL_NumJoysticks();
    for (u32 i = 0; i < input->num_of_joysticks; i++) {
        input->joysticks[i] = SDL_JoystickOpen(i);
        if (SDL_IsGameController(i)) {
            print("(sdl) %s\n", SDL_JoystickName(input->joysticks[i]));

            input->game_controllers[i] = SDL_GameControllerOpen(i);
            input->num_of_game_controllers++;
        }
    }
}

internal float32
sdl_process_stick_value(s16 value, s16 dead_zone) {
    float32 result = 0;
    if (value < -dead_zone) {
        result = float32((value + dead_zone) / (32768.0f - dead_zone));
    } else if (value > dead_zone) {
        result = float32((value - dead_zone) / (32767.0f - dead_zone));
    }
    return result;
}

internal bool8
sdl_process_input(App *app, App_Window *window, App_Input *input, SDL_Window *sdl_window) {
    //input->mouse = {};
    input->mouse_rel = {};
    input->buffer_index = 0;
    
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
    		switch(event.type) {
            case SDL_QUIT: {
                return true;
            } break;

            case SDL_WINDOWEVENT: {
                SDL_WindowEvent *window_event = &event.window;

                switch(window_event->event) {
                    //case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        window->resized = true;

                        window->width  = window_event->data1;
                        window->height = window_event->data2;
                        render_context.window_dim = window->dim;

                        render_context.update_resolution();
                    #ifdef OPENGL
                        //opengl_update_window(window);
                        render_context.resolution = window->dim;
                    #elif VULKAN
                    #elif DX12
                        dx12_resize_window(&dx12_renderer, window);
                    #endif // OPENGL / DX12
                        event_handler(app, APP_RESIZED, 0);
                    } break;

                    case SDL_WINDOWEVENT_MINIMIZED: {
                        app->window.minimized = true;
                    } break;

                    case SDL_WINDOWEVENT_RESTORED: {
                        app->window.minimized = false;
                    } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                SDL_MouseMotionEvent *mouse_motion_event = &event.motion;
                input->mouse.x = mouse_motion_event->x;
                input->mouse.y = mouse_motion_event->y;
                input->mouse_rel.x += mouse_motion_event->xrel;
                input->mouse_rel.y += mouse_motion_event->yrel;

                input->active = MOUSE_INPUT;
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                SDL_MouseButtonEvent *mouse_event = &event.button;
                event_handler(app, APP_MOUSEDOWN, mouse_event->button);
            } break;

            case SDL_MOUSEBUTTONUP: {
                SDL_MouseButtonEvent *mouse_event = &event.button;
                event_handler(app, APP_MOUSEUP, mouse_event->button);
            } break;

            case SDL_TEXTINPUT: {
                SDL_TextInputEvent *text_input_event = &event.text;
                app_copy_string_to_input_buffer(input, text_input_event->text);
            } break;

            case SDL_TEXTEDITING: {
                
                SDL_TextEditingEvent *text_edit_event = &event.edit;

            } break;

            case SDL_KEYDOWN: {
                SDL_KeyboardEvent *keyboard_event = &event.key;
                
                u32 key_id = keyboard_event->keysym.sym;
                bool32 ctrl = keyboard_event->keysym.mod & (KMOD_LCTRL | KMOD_RCTRL);
                
                if (!SDL_IsTextInputActive()) {
                    event_handler(app, APP_KEYDOWN, key_id);
                } else {
                    // text input
                    if (ctrl && key_id == SDLK_v) {
                        sdl_get_clipboard_text(input);
                        break;
                    }

                    // aligns with what the gui_textbox is expecting
                    s32 ch = 0;
                    switch(key_id) {
                        case SDLK_BACKSPACE: ch = 8; break;
                        case SDLK_RETURN: ch = 13; break;
                        case SDLK_LEFT: ch = 4437; break;
                        case SDLK_RIGHT: ch = 4439; break;
                        case SDLK_ESCAPE: ch = 27; break;
                    }
                                
                    if (ch != 0 && input->buffer_index < ARRAY_COUNT(input->buffer))
                        input->buffer[input->buffer_index++] = ch;
                }
            } break;

            case SDL_KEYUP: {
                SDL_KeyboardEvent *keyboard_event = &event.key;
                u32 key_id = keyboard_event->keysym.sym;
                if (key_id == SDLK_F11) {
                    app_toggle_fullscreen(window);
                    window->new_display_mode = true;
                    break;
                }
                event_handler(app, APP_KEYUP, key_id);
            } break;

            case SDL_CONTROLLERBUTTONDOWN: {
                SDL_ControllerButtonEvent *button_event = &event.cbutton;
                SDL_GameController *controller = SDL_GameControllerFromInstanceID(button_event->which);
                input->controller_type = SDL_GameControllerGetType(controller);
                
                event_handler(app, APP_CONTROLLER_BUTTONDOWN, button_event->button);
            } break;    
            
            case SDL_CONTROLLERBUTTONUP: {
                SDL_ControllerButtonEvent *button_event = &event.cbutton;
                SDL_GameController *controller = SDL_GameControllerFromInstanceID(button_event->which);
                input->controller_type = SDL_GameControllerGetType(controller);
                
                event_handler(app, APP_CONTROLLER_BUTTONUP, button_event->button);
            } break;    

            case SDL_CONTROLLERAXISMOTION: {
                SDL_ControllerAxisEvent *axis_event = &event.caxis;
                SDL_GameController *controller = SDL_GameControllerFromInstanceID(axis_event->which);
                input->controller_type = SDL_GameControllerGetType(controller);

                u32 button_low = 0;
                u32 button_high = 0;

                if (axis_event->axis == SDL_CONTROLLER_AXIS_LEFTX) {
                    button_low  = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
                    button_high = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
                } else if (axis_event->axis == SDL_CONTROLLER_AXIS_LEFTY) {
                    button_low  = SDL_CONTROLLER_BUTTON_DPAD_UP;
                    button_high = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
                } else {
                    break;
                }
                        
                float32 threshold = 0.5f;
                s16 dead_zone_threshold = 5000;
                float32 value = sdl_process_stick_value(axis_event->value, dead_zone_threshold);
                
                App_System_Event event_low = APP_CONTROLLER_BUTTONUP;
                App_System_Event event_high = APP_CONTROLLER_BUTTONUP;
                
                if ((value < -threshold) ? 1 : 0)
                    event_low = APP_CONTROLLER_BUTTONDOWN;
                else if ((value > threshold) ? 1 : 0)
                    event_high = APP_CONTROLLER_BUTTONDOWN;
                
                event_handler(app, event_low, button_low);
                event_handler(app, event_high, button_high);
            } break;
		    }
    }

    return false;
}

internal void
sdl_set_icon(Bitmap *icon, SDL_Window *sdl_window) {
    if (icon == 0) {
        logprint("(sdl) sdl_set_icon()", "no icon set\n");
        return;
    }
    
    SDL_Surface *icon_surface = SDL_CreateRGBSurfaceFrom(icon->memory, icon->dim.width, icon->dim.height, 32, icon->pitch, 0x00000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    SDL_SetWindowIcon(sdl_window, icon_surface);
    SDL_FreeSurface(icon_surface);
}

int main(int argc, char *argv[]) {
    print("(sdl) starting application...\n");
    
    App app = {};

    app.time.performance_frequency = SDL_GetPerformanceFrequency();
    app.time.start_ticks           = SDL_GetPerformanceCounter();
    app.time.last_frame_ticks      = app.time.start_ticks;
    
    u32 sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO;
    if (SDL_Init(sdl_init_flags)) {
        print(SDL_GetError());
        return 1;
    }

    qsock_init_qsock();

    u32 sdl_window_flags = SDL_WINDOW_RESIZABLE;
#ifdef OPENGL
    sdl_window_flags |= SDL_WINDOW_OPENGL;
#elif VULKAN
    sdl_window_flags |= SDL_WINDOW_VULKAN;
#elif DX12
    
#endif // OPENGL / DX12

    SDL_Window *sdl_window = SDL_CreateWindow("play_nine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1080, 720, sdl_window_flags);
    if (sdl_window == NULL) {
    	print(SDL_GetError());
    	return 1;
    }

#ifdef STEAM

    // overrides steam_appid.txt
    if (SteamAPI_RestartAppIfNecessary(480)) {
        return 1;
    }

    if (!SteamAPI_Init()) {
        logprint("main()", "SteamAPI_Init() failed\n" );
        return 1;
    }
    const char *name = SteamFriends()->GetPersonaName();

    Steam_Manager local_steam_manager;
    steam_manager = &local_steam_manager;
    
#endif // STEAM

    if (SDL_HasScreenKeyboardSupport() == SDL_TRUE)
        print("(sdl) Has Screen Keyboard Support!\n");
    
    SDL_GetWindowSize(sdl_window, &app.window.width, &app.window.height);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_StopTextInput();

    render_context.window_dim = app.window.dim;
    render_context.resolution = app.window.dim;
    
    if (render_sdl_init(sdl_window)) {
        logprint("(sdl) main()", "Failed to init renderer\n");
        return 1;
    }
    
    if (event_handler(&app, APP_INIT, 0))
        return 1;

    sdl_set_icon(app.icon, sdl_window);

    SDL_Input sdl_input = {};
    sdl_init_controllers(&sdl_input);
    
    srand(SDL_GetTicks());

    if (app.update == 0) {
        logprint("(sdl) main()", "no update function set\n");
        return 1;
    }

    while (1) {
        if (app.window.new_display_mode) {
            app.window.new_display_mode = false;
            sdl_set_fullscreen(sdl_window, app.window.display_mode);
        }
        
        if (app.input.relative_mouse_mode) 
            SDL_SetRelativeMouseMode(SDL_TRUE);
        else 
            SDL_SetRelativeMouseMode(SDL_FALSE);

#ifdef STEAM

        SteamAPI_RunCallbacks();
        
#endif // STEAM

        if (sdl_process_input(&app, &app.window, &app.input, sdl_window)) 
            break;
        
        sdl_update_time(&app.time);
        //print("%f\n", app.time.frames_per_s);
        //print("%f\n", app.time.run_time_s);

        if (app.window.minimized)
            continue;

        if (app.update(&app))
            break;
    }

    render_wait_frame();
    event_handler(&app, APP_EXIT, 0) ;
    render_cleanup();

#ifdef STEAM

    SteamAPI_Shutdown();

#endif // STEAM
    
    SDL_DestroyWindow(sdl_window);

    return 0;
}
