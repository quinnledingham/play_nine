struct Card_Entity {
    u32 index;
    bool8 hovered;
    Transform transform;
};

struct Game_Draw {
    bool8 enabled = false;

    float32 hand_width;
    Vector2 card_dim;
    Vector2 relative_hand_coords[HAND_SIZE];
    Vector3 absolute_hand_coords[MAX_PLAYERS];
    float32 player_hand_rads[MAX_PLAYERS];
    float32 degrees_between_players;
    float32 radius; // from 0,0,0 to player hand position

    float32 pile_distance_from_hand = -6.7f;
    float32 x_hand_position; // where hand on x_axis is location

    const Vector4 card_side_color = {90, 90, 90, 1};

    float32 rotation;

    ArrayB<Card_Entity> card_entities;
    Mesh hitbox;

    Card_Entity *draw_pile_entity;
    Card_Entity *discard_pile_entity;
    Card_Entity *picked_up_card_entity;

    Vector3 draw_pile_offset      = { -1.0f,    0,     0 };
    Vector3 discard_pile_offset   = {  1.0f,    0,     0 };
    Vector3 picked_up_card_offset = {     0, 1.0f, -3.7f };
};

global const Vector4 play_nine_green        = {  39,  77,  20, 1 };
global const Vector4 play_nine_yellow       = { 231, 213,  36, 1 };
global const Vector4 play_nine_light_yellow = { 240, 229, 118, 1 };
global const Vector4 play_nine_dark_yellow  = { 197, 180,  22, 1 };

internal Pose get_player_camera(float32 degrees_between, u32 active_i);
internal Pose get_pile_pose(Game *game, Game_Draw *draw, Vector3 offset, float32 omega);

// Card Ratio: 63/88

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