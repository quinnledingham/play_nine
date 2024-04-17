/*
TODO
- Singleplayer (Bot)

- Flip "animation"
- Show keyboard controls
- turn off add player for online?
- see all cards in larger lobby's in online

Slight Problems
- Improve face cards
- Improve game hud

Render.cpp
- Clean up light sources in shaders
- Add wireframe and vsync toggle
- Fix "skybox" shading
*/

#define PICKUP_PILE    8
#define DISCARD_PILE   9
#define PASS_BUTTON   10
#define SELECTED_SIZE 11

#define MAX_NAME_SIZE  20
#define MAX_PLAYERS     6
#define HAND_SIZE       8
#define DECK_SIZE     108
#define MAX_HOLES      20 // max holes that can be played in one gamed
#define GAME_LENGTH     2 // play NINE

global s8 deck[DECK_SIZE];
global Bitmap card_bitmaps[14];
global Vector2 hand_coords[8];
global float32 hand_width;

global Font *default_font;
Draw_Style default_style = {};

global const Vector4 play_nine_green  = {  39,  77,  20, 1 };
global const Vector4 play_nine_yellow = { 231, 213,  36, 1 };
global const Vector4 play_nine_light_yellow = { 240, 229, 118, 1 };
global const Vector4 play_nine_dark_yellow = { 197, 180, 22, 1 };

const Vector4 highlight_colors[3] = { play_nine_yellow, play_nine_light_yellow, play_nine_dark_yellow };

Assets *global_assets;
Descriptor texture_desc = {}; // card textures

Render_Pipeline basic_pipeline;
Render_Pipeline text_pipeline;
Compute_Pipeline ray_pipeline;

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

struct Player {
    char name[MAX_NAME_SIZE];
    s32 scores[MAX_HOLES];

    u32 cards[HAND_SIZE];     // indices to global deck array
    bool8 flipped[HAND_SIZE];

    bool8 is_bot;
};

// Stores information about the game of play nine
struct Game {
    // Entire Game variables
    Player players[MAX_PLAYERS];
    u32 num_of_players;
    u32 active_player;

    u32 starting_player;
    u32 holes_played;
    bool8 game_over;

    // Hole variables
    u32 pile[DECK_SIZE];
    u32 top_of_pile;

    u32 discard_pile[DECK_SIZE];
    u32 top_of_discard_pile;

    enum Round_Types round_type;
    u32 last_turn;

    // Turn variables
    u32 new_card; // the card that was just picked up
    bool8 pile_card; // flag to flip if discard new card
    enum Turn_Stages turn_stage;

    // Bot
    float32 bot_thinking_time;
};

//
// State
//

enum Camera_Mode {
    FREE_CAMERA,
    PLAYER_CAMERA,
};

struct Rotation {
    bool8 signal;
    bool8 rotating;
    
    float32 degrees;      // how much to rotate the object
    float32 dest_degrees;

    float32 speed;
};

struct Game_Draw {
    Rotation pile_rotation;
    Rotation camera_rotation;

    float32 rotation_speed;
    float32 rotation; // for in hole over state
    Vector2_s32 mouse_down;

    float32 degrees_between_players;
    float32 radius;

    bool8 highlight_hover[SELECTED_SIZE];
    bool8 highlight_pressed[SELECTED_SIZE];

    Bitmap *bot_bitmap;
    Bitmap name_plates[MAX_PLAYERS];

    Matrix_4x4 new_card_model;
    Matrix_4x4 top_of_pile_model;
    Matrix_4x4 top_of_discard_pile_model;
    Matrix_4x4 hand_models[MAX_PLAYERS][HAND_SIZE];
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

    u32 previous_menu;
    Menu_List menu_list;

    // Server
    char name[TEXTBOX_SIZE] = "Jeff";
    char ip[TEXTBOX_SIZE] = "127.0.0.1";
    char port[TEXTBOX_SIZE] = "4444";

    bool8 is_server;
    bool8 is_client;
    QSock_Socket client;
    u32 client_game_index; // what player index in game for the local player

    bool8 is_active; // is currently taking inputs (local game, active player online client, not a bot)

    bool8 selected[SELECTED_SIZE];
    s64 selected_mutex;

    bool8 pass_selected;
    
    // Drawing
    Scene scene;
    Scene ortho_scene;
    Descriptor scene_set;
    Descriptor scene_ortho_set;
    
    Camera camera;
    s32 indices[16];

    Assets assets;
};

// 0 1 2 3 4 5 6 7 8 9 10 11 12 -5
Vector3 ball_colors[14] = {
    { 255, 255, 255 }, // 0
    { 129, 125,  21 }, // 1 yellow 
    { 206,  69,  64 }, // 2 red
    { 158,  16, 125 }, // 3 purple
    {  38, 146, 130 }, // 4 teal
    { 178, 206,  64 }, // 5 lime green
    { 206,  64, 130 }, // 6 pink

    {  39,  77,  20 }, // 7
    play_nine_yellow.xyz, // 8 { 255, 213,  36 }
    {  49,  20,  77 }, // 9 dark purple
    {  20,  48,  77 }, // 10 dark blue
    { 255, 100,   0 }, // 11
    {   0, 255, 100 }, // 12
    { 255,   0,   0 }  // 13 Full red
};

s32 rows[14][3] = {
    { 0, 0, 0 }, // 0

    { 0, 1, 0, }, // 1
    { 0, 2, 0, }, // 2
    { 0, 3, 0, }, // 3

    { 2, 0, 2, }, // 4
    { 2, 1, 2, }, // 5
    { 3, 0, 3, }, // 6

    { 3, 1, 3, }, // 7
    { 3, 2, 3, }, // 8
    { 3, 3, 3, }, // 9

    { 4, 2, 4, }, // 10
    { 0, 1, 0, }, // 11
    { 0, 1, 0, }, // 12

    { 2, 1, 2, }, // -5
};
