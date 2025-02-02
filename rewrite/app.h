#ifndef APP_H
#define APP_H

/*
TODO:

Handle Input Better
Expect input from either keyboard, mouse, controller all the time
Change what is shown by the active device
Change the type of input based on the state of the game (have on screen buttons)

*/

enum Input_Types {
  INPUT_TYPE_KEYBOARD,
  INPUT_TYPE_MOUSE,
  INPUT_TYPE_CONTROLLER,

  INPUT_TYPE_COUNT
};

struct App_Input {

};

struct App_Time {
	s64 start_ticks;
	s64 last_frame_ticks;
	s64 performance_frequency;

	float64 run_time_s;
	float64 frame_time_s;

	float64 frames_per_s;
};

struct App_Audio_Player {
  bool8 playing;
};

struct App {
  App_Input input;
  App_Time time;
  App_Audio_Player audio_player;
  Assets assets;

  bool8 (*update)();
};

enum App_Events {
  EVENT_INIT,
  EVENT_QUIT,

  EVENT_COUNT
};

App app = {};

#endif // APP_H