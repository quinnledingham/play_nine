/*
TODO
- Singleplayer (Bot)

- Highlight card choices
- Flip "animation"

- Online

- Fix opengl support

- Pack assets

*/

#define PICKUP_PILE 8
#define DISCARD_PILE 9
#define SELECTED_SIZE 10

struct Card {
	s32 number;
    Matrix_4x4 model;
};

#define MAX_NAME_SIZE 20
#define HAND_SIZE 8
#define DECK_SIZE 108
global Card deck[DECK_SIZE];

enum Round_Types {
    FLIP_ROUND,
    REGULAR_ROUND,
    FINAL_ROUND,
    HOLE_OVER
};

const char *round_types[4] = {
    "Flip Round",
    "Regular Round",
    "Final Round",
    "Hole Over"
};

enum Turn_Stages {
    SELECT_PILE, // pick either pile or discard pile
    SELECT_CARD, // pick where to place new card - in hand or discard
    FLIP_CARD,   // flip card if discard new card
};

/*

Card indices

0 1 2 3
4 5 6 7

*/

#define MAX_HOLES 20 // max holes that can be played in one gamed
#define GAME_LENGTH 2 // play NINE

struct Player {
    char name[MAX_NAME_SIZE];
    s32 scores[MAX_HOLES];

    u32 cards[HAND_SIZE];     // indices to global deck array
    bool8 flipped[HAND_SIZE];
    u32 new_card; // the card that was just picked up
    bool8 pile_card; // flag to flip if discard new card
    enum Turn_Stages turn_stage;
};

enum Game_Modes {
    LOBBY,
    SCOREBOARD,
    GAME,
};

// Stores information about the game of play nine
struct Game {
	u32 pile[DECK_SIZE];
	u32 top_of_pile;

    u32 discard_pile[DECK_SIZE];
    u32 top_of_discard_pile;

    Player players[6];
    u32 num_of_players;
    u32 active_player;

    u32 holes_played;
    bool8 game_over;

    enum Round_Types round_type;
    u32 last_turn;
};

//
// State
//

enum Camera_Mode {
    FREE_CAMERA,
    PLAYER_CAMERA,
};

global Bitmap card_bitmaps[14];
global Vector2 hand_coords[8];
global float32 hand_width;

struct Rotation {
    bool8 rotating;
    
    float32 degrees;      // how much to rotate the object
    float32 dest_degrees;

    float32 speed;
};

struct Game_Draw {
    Rotation pile_rotation;
    Rotation camera_rotation;

    float32 rotation_speed;

    float32 degrees_between_players;
    float32 radius;
};

enum Menu_Mode {
    MAIN_MENU,
    LOCAL_MENU,
    PAUSE_MENU,
    SCOREBOARD_MENU,

    HOST_MENU,
    JOIN_MENU,

    IN_GAME
};

struct Menu_List {
    enum Menu_Mode mode = MAIN_MENU;

    union {
        struct {
            Menu main, local, pause, scoreboard, host, join;
        };
        Menu menus[6];
    };
};

struct State {
    s64 mutex;

    Game game;
    Game_Draw game_draw;
    Onscreen_Notifications notifications;

    // Input
    Controller controller = {};
    enum Camera_Mode camera_mode = PLAYER_CAMERA;
    Ray mouse_ray;

    Menu_List menu_list;

    // Server
    char name[TEXTBOX_SIZE];
    char ip[TEXTBOX_SIZE];
    char port[TEXTBOX_SIZE];

    bool8 is_server;

    bool8 is_client;
    QSock_Socket client;
    u32 client_game_index;

    bool8 selected[SELECTED_SIZE];
    s64 selected_mutex;

    // Drawing
    Scene scene;
    Scene ortho_scene;
    Descriptor_Set *scene_set;
    Descriptor_Set *scene_ortho_set;
    
    Camera camera;

    Assets assets;
};

global Font *default_font;
global const Vector4 play_nine_green  = {  39,  77,  20, 1 };
global const Vector4 play_nine_yellow = { 231, 213,  36, 1 };
global const Vector4 play_nine_light_yellow = { 240, 229, 118, 1 };
global const Vector4 play_nine_dark_yellow = { 197, 180, 22, 1 };