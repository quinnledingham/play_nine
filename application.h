#ifndef APPLICATION_H
#define APPLICATION_H


struct Application_Window {
	s32 width;
	s32 height;
	float32 aspect_ratio;
};

struct Application_Time {
	s64 start_ticks;
	s64 last_frame_ticks;
	s64 performance_frequency;

	float32 run_time_s;
	float32 frame_time_s;

	float32 frames_per_s;
};

inline float64
get_seconds_elapsed(Application_Time *time, s64 start, s64 end) {
    float64 result = ((float64)(end - start) / (float64)time->performance_frequency);
    return result;
}

struct Application_Input {

};

struct Application {
	Application_Window window;
	Application_Time time;
	Application_Input input;

	void *data;
};

#endif // APPLICATION_H