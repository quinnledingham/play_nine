//
// General Draw
//

#define CARD_MODEL_HEIGHT 0.101767f

global Bitmap card_bitmaps[14];
global Vector2 hand_coords[8];
global float32 hand_width;

global const Vector4 play_nine_green        = {  39,  77,  20, 1 };
global const Vector4 play_nine_yellow       = { 231, 213,  36, 1 };
global const Vector4 play_nine_light_yellow = { 240, 229, 118, 1 };
global const Vector4 play_nine_dark_yellow  = { 197, 180,  22, 1 };

const Vector4 highlight_colors[3] = { play_nine_yellow, play_nine_light_yellow, play_nine_dark_yellow };

global Font *default_font;
Draw_Style default_style = {};

Descriptor texture_desc = {}; // card textures

struct Rotation {
    bool8 signal;
    bool8 rotating;
    
    float32 degrees;      // how much to rotate the object
    float32 dest_degrees;

    float32 speed;
};

struct Card_Animation_Keyframe {
    Vector3 start;
    Vector3 dest;

    float32 time_elapsed;
    float32 time_duration;

    bool8 dynamic;
    Vector3 *dynamic_dest;

    void fill(Vector3 in_start, Vector3 in_dest, float32 duration) {
        start = in_start;
        dest = in_dest;
        time_duration = duration;
        time_elapsed = 0.0f;
    };
    void fill(Vector3 in_start, Vector3 *in_dest, float32 duration) {
        dynamic = true;
        start = in_start;
        dynamic_dest = in_dest;
        time_duration = duration;
        time_elapsed = 0.0f;
    };
};

struct Card_Animation {
    bool8 in_use;

    Vector3 position;
    float32 y_axis_rad;
    float32 z_axis_rad;
    Matrix_4x4 *model;

    bool8 dynamic;
    float32 *dynamic_y_axis_rad;

    bool8 moving;
    
    u32 keyframes_count;
    Card_Animation_Keyframe keyframes[10];
    
    Rotation rotation; // z_axis;

    void add_movement(Vector3 in_start, Vector3 in_dest, float32 duration) {
        moving = true;

        keyframes_count = 1;
        keyframes[0].fill(in_start, in_dest, duration);
    }
    
    void add_movement(Vector3 in_start, Vector3 in_middle, Vector3 in_dest, float32 length) {
        moving = true;

        keyframes_count = 2;
        keyframes[0].fill(in_start, in_middle, length / 2.0f);
        keyframes[1].fill(in_middle, in_dest, length / 2.0f);        
    }
    
    void add_movement(Vector3 in_start, Vector3 in_middle, Vector3 *in_dest, float32 length) {
        moving = true;

        keyframes_count = 2;
        keyframes[0].fill(in_start, in_middle, length / 2.0f);
        keyframes[1].fill(in_middle, in_dest, length / 2.0f);        
    }

    void add_rotation(float32 start, float32 dest, float32 speed) {
        rotation.rotating = true;
        rotation.degrees = start;
        rotation.dest_degrees = dest;
        rotation.speed = speed;
    }
};

struct Game_Draw_Info {
    float32 rotation_speed = 150.0f;
    float32 card_flipping_speed = 500.0f;
    float32 degrees_between_players;
    float32 radius;
    Vector3 card_scale = {1.0f, 0.5f, 1.0f};
    float32 pile_distance_from_hand = 5.7f;
    
    Vector3 pile_coords;
    Vector3 discard_pile_coords;
    Vector3 selected_card_coords;
    float32 pile_rads;
    
    float32 x_hand_position; // where hand on x_axis is location
    float32 player_hand_rads[MAX_PLAYERS];
    Vector3 hand_positions[MAX_PLAYERS];
};

struct Game_Draw {
    Game_Draw_Info info;
    
    Rotation camera_rotation; // rotation when switching players, also controls pile rotation
    float32 rotation; // for in HOLE_OVER state
    Vector2_s32 mouse_down;

    bool8 highlight_hover[SELECTED_SIZE];
    bool8 highlight_pressed[SELECTED_SIZE];

    Bitmap *bot_bitmap;
    Bitmap name_plates[MAX_PLAYERS];

    bool8 moving_card;
    Matrix_4x4 moving_card_model;
    Matrix_4x4 selected_card_model;
    Matrix_4x4 top_of_pile_model;
    Matrix_4x4 top_of_discard_pile_model;
    Matrix_4x4 hand_models[MAX_PLAYERS][HAND_SIZE];

    Rotation new_card_rotation;
    Rotation card_rotation[MAX_PLAYERS][HAND_SIZE]; // flip
    //bool8 flipping[HAND_SIZE];

    static const u32 max_card_animations = 20;
    u32 animations_count;
    Card_Animation animations[max_card_animations];
};

//
// Card Designs
// color and layout of the circles on the card bitmaps
//

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

s32 ball_rows[14][3] = {
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

//
// Draw_Signal
//

enum Draw_Signal_Types {
    NO_SIGNAL,
    SIGNAL_ALL_PLAYER_CARDS,
    SIGNAL_FLIP_CARD,
    SIGNAL_PILE_CARDS,
    SIGNAL_NEXT_PLAYER_ROTATION,
    SIGNAL_NEW_CARD_FROM_PILE,
    SIGNAL_NEW_CARD_FROM_DISCARD,
    SIGNAL_DISCARD_SELECTED,
    SIGNAL_REPLACE,
    SIGNAL_NAME_PLATES,
    SIGNAL_UNLOAD_NAME_PLATE,
};

struct Draw_Signal {
    u32 type;
    u32 card_index;
    u32 player_index;
    bool8 flipped;
    bool8 fulfilled[6]; // tracking which clients have completed the signal
    bool8 in_use; // if this signal is still active
    bool8 sent;

    Draw_Signal() {
        type = 0;
        card_index = 0;
        player_index = 0;
        platform_memory_set(fulfilled, 0, sizeof(bool8) * 6);
        in_use = 0;
        sent = 0;
    }
    Draw_Signal(u32 in_type, u32 in_card_index, u32 in_player_index) {
        type = in_type;
        card_index = in_card_index;
        player_index = in_player_index;
        platform_memory_set(fulfilled, 0, sizeof(bool8) * 6);
        in_use = true;
        sent = 0;
    }
};

// Draw Signals: so that draw updates don't have to happen in the update functions
internal void add_draw_signal(Draw_Signal *signals, u32 in_type, u32 in_card_index, u32 in_player_index, bool8 flipped);
internal void add_draw_signal(Draw_Signal *signals, u32 in_type, u32 in_card_index, u32 in_player_index);
internal void add_draw_signal(Draw_Signal *signals, u32 in_type);
internal void add_draw_signal(Draw_Signal *signals, Draw_Signal s);

Draw_Signal draw_signals[DRAW_SIGNALS_AMOUNT];
