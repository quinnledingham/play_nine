#define DECK_SIZE 108
#define MAX_PLAYERS 6
#define MAX_HOLES 20
#define HAND_SIZE 8

enum Round_Types {
    FLIP_ROUND,
    REGULAR_ROUND,
    FINAL_ROUND,
    HOLE_OVER,
};

enum Turn_Stages {
    SELECT_PILE,
    SELECT_CARD,
    FLIP_CARD,
};

enum Game_Input {
    GI_0,
    GI_1,
    GI_2,
    GI_3,

    GI_4,
    GI_5,
    GI_6,
    GI_7,

    GI_DRAW_PILE,
    GI_DISCARD_PILE,

    GI_PASS_BUTTON,

    GI_SIZE
};

/*

Card indices

0 1 2 3
4 5 6 7

*/

struct Player_Card {
    u8 index;
    bool8 flipped;
};

struct Player {
    Player_Card cards[HAND_SIZE];

    u8 replace_card(u8 card_index, u8 picked_up_card) {
        Player_Card *card = &cards[card_index];
        u8 discarded = card->index;
        card->index = picked_up_card;
        card->flipped = true;
        return discarded;
    }
};

struct Player_Turn {
    u8 new_card; // != 0 if from a not from the discard pile
    s8 picked_up_card = -1; // what card was picked up from the piles
    enum Turn_Stages stage;
};

struct Card_Pile {
    u8 indices[DECK_SIZE];
    u8 top;

    u8 top_card() {
        return indices[top - 1];
    }

    u8 draw_card() {
        u8 index = indices[top - 1];
        top--;
        return index;
    }

    bool8 empty() {
        if (top == 0)
            return true;
        else
            return false;
    }

    void add_card(u8 index) {
        indices[top++] = index;
    }

    void reset() {
        memset(indices, 0, sizeof(u8) * DECK_SIZE);
        top = 0;
    }
};

struct Game {
    Player players[MAX_PLAYERS];
    u8 players_count;
    u8 active_player;

    u8 starting_player; // tracks who should start the hole
    u8 holes_played;
    u8 holes_length = 9;
    bool8 game_over;

    enum Round_Types round_type;
    u32 last_turn;

    Player_Turn turn;

    Card_Pile draw_pile;
    Card_Pile discard_pile;
};
