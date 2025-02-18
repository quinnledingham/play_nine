#define MAX_NAME_SIZE  20
#define MAX_PLAYERS     6
#define HAND_SIZE       8
#define DECK_SIZE     108
// max holes that can be played in one gamed
#define MAX_HOLES      20 

enum Game_Input {
    GI_0,
    GI_1,
    GI_2,
    GI_3,

    GI_4,
    GI_5,
    GI_6,
    GI_7,

    GI_PICKUP_PILE,
    GI_DISCARD_PILE,

    GI_PASS_BUTTON,

    GI_SIZE
};

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
    s32 total_score;

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

    u32 starting_player; // tracks who should start the hole
    s32 holes_length = 9; // number of holes
    u32 holes_played;
    bool8 game_over;

    // Hole variables
    u32 pile[DECK_SIZE];
    u32 top_of_pile; // index in pile of the top card

    u32 discard_pile[DECK_SIZE];
    u32 top_of_discard_pile; // index in discard pile of the top card

    enum Round_Types round_type;
    u32 last_turn; // counting how many players have had their last turn

    // Turn variables
    u32 new_card; // the card that was just picked up
    bool8 pile_card; // flag to flip if discard new card
    enum Turn_Stages turn_stage;
    
    // Bot
    float32 bot_thinking_time;
    
    float32 turn_time;
};
