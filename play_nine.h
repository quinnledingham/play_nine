/*
TODO
- More music/sounds
- Show keyboard controls
- Skybox
- Multithreading

- Controller
- improve how controller moves hover around in game
- make it easier to see what card is hovered
- change hud buttons to use other controller buttons
- controller with textboxs? (number of holes / ip / port)

Scoreboard
- make scoreboard look better
- add scrolling to scoreboard

- (gui_textbox) add undoing and select all
- add victory screen
- show hole scores on hud

Assets
- Put mtl and map_Kd files into asset file

Bot
- Add look ahead, so the bot doesnt give pairs to next player
- Replace highest card, not just first card higher than threshold

Slight Problems
- Improve face cards
- Improve game hud

Render.cpp
- Add wireframe toggle
- add target frame rate

vulkan.h
- make sure surface formats are loading correctly (target and order on gpu)
*/

enum Direction {
    DIRECTION_NONE,
    
    RIGHT,
    UP,
    LEFT,
    DOWN,

    DIRECTION_SIZE
};

enum Menu_Mode {
    MAIN_MENU,
    LOCAL_MENU,
    PAUSE_MENU,
    SCOREBOARD_MENU,

    HOST_MENU,
    JOIN_MENU,

    SETTINGS_MENU,
    VIDEO_SETTINGS_MENU,
    AUDIO_SETTINGS_MENU,

    IN_GAME,

    MENU_MODES_COUNT
};

struct Menu_List {
    enum Menu_Mode previous_mode; // can save a mode here
    enum Menu_Mode mode;

    Menu menus[MENU_MODES_COUNT];

    void update(enum Menu_Mode new_mode) {
        mode = new_mode;
    }

    void update_close(enum Menu_Mode new_mode) {
        menus[mode].gui.close_at_end = true;
        mode = new_mode;
    }

    void toggle(enum Menu_Mode mode_1, enum Menu_Mode mode_2) {
        if (mode == mode_1)
            update_close(mode_2);
        else
            update_close(mode_1);
    }
};

enum Camera_Mode {
    PLAYER_CAMERA,
    FREE_CAMERA
};

enum Online_Mode {
    MODE_LOCAL,
    MODE_CLIENT,
    MODE_SERVER
};

struct Loading_Icon {
    bool8 enabled;
    Bitmap *bitmap;
    float32 rotation;

    void enable(Bitmap *in_bitmap);
    void disable();
    void draw(Vector2_s32 window_dim);
};

struct State {
    MUTEX mutex;

    Game game;
    Game_Draw game_draw;
    Onscreen_Notifications notifications;

    char num_of_holes[TEXTBOX_SIZE];
    
    // Input
    Controller controller = {};
    enum Camera_Mode camera_mode = PLAYER_CAMERA;

    Menu_List menu_list;

    // Host/Join Menu textboxes
    char join_name[TEXTBOX_SIZE] = "Jeff";
    char join_ip[TEXTBOX_SIZE] = "127.0.0.1";
    char join_port[TEXTBOX_SIZE] = "4444";
    char host_port[TEXTBOX_SIZE] = "4444";

    enum Online_Mode mode;    
    u32 client_game_index; // what player index in game for the local player

    bool8 is_active; // is currently taking inputs (local game, active player online client, not a bot)

    // online selected received from client
    bool8 selected[GI_SIZE];
    MUTEX selected_mutex;

    bool8 pass_selected;

    Descriptor triangle_desc;
    
    // Drawing
    Scene scene;
    Scene ortho_scene;
    Descriptor scene_set;
    Descriptor scene_ortho_set;
    
    Camera camera;

    Loading_Icon loading_icon;
    
    Assets assets;
};

enum Online_Mode *global_mode;
global s8 deck[DECK_SIZE];
GUI gui = {};

global Texture_Atlas atlas;
