#ifndef APPLICATION_H
#define APPLICATION_H

enum Display_Modes {
	DISPLAY_MODE_WINDOWED,
	DISPLAY_MODE_FULLSCREEN,
	DISPLAY_MODE_WINDOWED_FULLSCREEN,
};

struct App_Window {
	union {
		struct {
			s32 width;
			s32 height;
		};
		Vector2_s32 dim;
	};
	float32 aspect_ratio;
	bool8 resized;
	bool8 minimized;
	enum Display_Modes display_mode;
};

internal void
app_toggle_fullscreen(App_Window *window) {
	switch(window->display_mode) {
	    case DISPLAY_MODE_WINDOWED: window->display_mode = DISPLAY_MODE_FULLSCREEN; break;
	    case DISPLAY_MODE_WINDOWED_FULLSCREEN:
	    case DISPLAY_MODE_FULLSCREEN: window->display_mode = DISPLAY_MODE_WINDOWED; break;
	}
}

global Vector2_s32 window_dim;

struct App_Time {
	s64 start_ticks;
	s64 last_frame_ticks;
	s64 performance_frequency;

	float64 run_time_s;
	float64 frame_time_s;

	float64 frames_per_s;
};

inline float64
get_seconds_elapsed(App_Time *time, s64 start, s64 end) {
    float64 result = ((float64)(end - start) / (float64)time->performance_frequency);
    return result;
}

struct App_Key_Event {
	s32 id;
	bool8 state; // true if pressed, false if released
};

enum Input_Type {
	KEYBOARD_INPUT,
	MOUSE_INPUT,
	CONTROLLER_INPUT,
};

bool8 app_input_buffer;

struct App_Input {
	Vector2_s32 mouse;
	Vector2_s32 mouse_rel;
	bool8 relative_mouse_mode = false;

    s32 buffer[10];
    s32 buffer_index;

	enum Input_Type active; // what was the last type of input used
};

typedef enum {
	APP_INIT,
	APP_EXIT,
	APP_KEYDOWN,
	APP_KEYUP,

	APP_RESIZED,

	APP_MOUSEDOWN,
	APP_MOUSEUP,
} App_System_Event;

struct App {
	App_Window window;
	App_Time time;
	App_Input input;
	Bitmap *icon;

	bool8 (*update)(App *app);

	void *data;
};

s32 event_handler(App *app, App_System_Event event, u32 arg);

#endif // APPLICATION_H
