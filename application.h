#ifndef APPLICATION_H
#define APPLICATION_H

struct App_Window {
	s32 width;
	s32 height;
	float32 aspect_ratio;
	bool8 resized;
};

struct App_Time {
	s64 start_ticks;
	s64 last_frame_ticks;
	s64 performance_frequency;

	float32 run_time_s;
	float32 frame_time_s;

	float32 frames_per_s;
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

struct App_Input {
	Vector2_s32 mouse;
	Vector2_s32 mouse_rel;
	bool8 relative_mouse_mode = true;

	static const u32 keys_buffer_size = 10;
	App_Key_Event key_events[keys_buffer_size];
	u32 key_events_count;
};

typedef enum {
	APP_INIT,
	APP_KEYDOWN,
	APP_KEYUP,
} App_System_Event;

struct App {
	App_Window window;
	App_Time time;
	App_Input input;

	bool8 (*update)(App *app);

	void *data;
};

s32 event_handler(App *app, App_System_Event event, u32 arg);

#endif // APPLICATION_H