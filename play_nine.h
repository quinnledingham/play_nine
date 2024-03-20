/*
TODO
- Singleplayer (Bot)

- Highlight card choices
- Flip "animation"

- Textbox

- Online

*/

struct Card {
	s32 number;
	
    const char *name; // i.e. mulligan, birdie
    bool8 flipped;
    Matrix_4x4 model;

    bool8 selected;

	// Drawing
	//Bitmap front;
	//Bitmap back;
	//Mesh mesh;
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
    u32 cards[HAND_SIZE];
    u32 new_card; // the card that was just picked up
    bool8 pile_card; // flag to flip if discard new card
    enum Turn_Stages turn_stage;

    s32 scores[MAX_HOLES];
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

    Onscreen_Notifications notifications;
};

enum Game_Mode {
    MAIN_MENU,
    LOCAL_MENU,
    PAUSE_MENU,
    SCOREBOARD_MENU,

    IN_GAME
};

struct Menu_List {
    enum Game_Mode mode = MAIN_MENU;

    union {
        struct {
            Menu main, local, pause, scoreboard;
        };
        Menu menus[4];
    };
};

struct State {
    Game game;
    Game_Draw game_draw;

    // Input
    Controller controller = {};
    enum Camera_Mode camera_mode = PLAYER_CAMERA;
    Ray mouse_ray;

    Menu_List menu_list;

    // Drawing
    Scene scene;
    Scene ortho_scene;
    Descriptor_Set *scene_set;
    Descriptor_Set *scene_ortho_set;
    
    Camera camera;

    Assets assets;
};

global Font *default_font;