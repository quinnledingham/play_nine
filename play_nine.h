/*
TODO
- Write inverse/determinant functions
- Mouse Controls
- Highlight card choices
- Flip "animation"
- Reset game
- Scoreboard
- On screen announcements

- Pause menu
- Textbox

- Online
*/

struct Card {
	s32 number;
	const char *name; // i.e. mulligan, birdie
	bool8 flipped;

	// Drawing
	//Bitmap front;
	//Bitmap back;
	//Mesh mesh;
};

#define DECK_SIZE 108
global Card deck[DECK_SIZE];
global Bitmap card_bitmaps[14];
global Vector2 hand_coords[8];

enum Round_Types {
    FLIP_ROUND,
    REGULAR_ROUND,
    FINAL_ROUND,
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
    const char *name;
    u32 cards[8];
    u32 new_card; // the card that was just picked up
    bool8 pile_card; // flag to flip if discard new card
    enum Turn_Stages turn_stage;
};

struct Rotation {
    bool8 rotating;
    
    float32 degrees;      // how much to rotate the object
    float32 dest_degrees;

    float32 speed;
};

struct Game {
	u32 pile[DECK_SIZE];
	u32 top_of_pile;

    u32 discard_pile[DECK_SIZE];
    u32 top_of_discard_pile;

    Player players[6];
    u32 num_of_players;
    u32 active_player;

    enum Round_Types round_type;

    // Game Draw info
    Rotation pile_rotation;
    Rotation camera_rotation;

    float32 rotation_speed;

    float32 degrees_between_players;
    float32 radius;
};

//
// State
//

struct Button {
    s32 id;
    
    s32 ids[3];
    u32 num_of_ids;
    
    bool8 current_state; 
    bool8 previous_state;
};

inline void 
set(Button *button, s32 id)  {
    if (button->num_of_ids > 2)
        logprint("set()", "too many ids trying to be assigned to button\n");
    
    button->ids[button->num_of_ids++] = id;
    button->id = id; 
}

inline bool8 
is_down(Button button) { 
    if (button.current_state) 
        return true;
    return false; 
}

inline bool8 
on_down(Button button) {
    if (button.current_state && button.current_state != button.previous_state) 
        return true;
    return false;
}

struct Controller {
    union {
        struct {
            // Game Controls

            Button one;
            Button two;
            Button three;
            Button four;
            
            Button five;
            Button six;
            Button seven;
            Button eight;

            Button nine;
            Button zero;

            // Camera Controls

            Button forward;
            Button backward;
            Button left;
            Button right;
            Button up;
            Button down;

            Button pause;
            Button select;
        };
        Button buttons[18];
    };
};

enum Camera_Mode {
    FREE_CAMERA,
    PLAYER_CAMERA,
};

enum Game_Mode {
    MAIN_MENU,
    LOCAL,
    ONLINE,
};

Render_Pipeline basic_pipeline;
Render_Pipeline color_pipeline;

struct State {
    Game game;

    Controller controller = {};
    enum Camera_Mode camera_mode = PLAYER_CAMERA;

    // Menus
    enum Game_Mode mode = MAIN_MENU;
    s32 active;

    Scene scene;
    Scene ortho_scene;
    Descriptor_Set *scene_set;
    Descriptor_Set *scene_ortho_set;
    Camera camera;
    Mesh rect;

    Bitmap test;

    Assets assets;
};

//
// Raycasting
// 

struct Ray {
    Vector3 origin;
    Vector3 direction;
};

struct Ray_Intersection {
    u32 number_of_intersections;
    Vector3 point;
    Vector3 normal;
};