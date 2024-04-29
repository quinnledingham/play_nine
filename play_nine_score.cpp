
struct Card_Pair {
    bool8 matching;
    s32 score;
    s32 number;
};

inline Card_Pair
create_pair(u32 top, u32 bottom) {
    Card_Pair result = {};
    s32 top_card = deck[top];
    s32 bottom_card = deck[bottom];

    if (top_card == bottom_card) {
        result.matching = true;
        result.number = top_card; // matching so top and bottom same number
    }

    if (!result.matching || (top_card == -5))
        result.score += top_card + bottom_card;

    return result;
} 

struct Card_Pair_Match {
    s32 number;
    s32 pairs;
};

internal s32
get_score(u32 *cards) {
    s32 score = 0;

    Card_Pair pairs[4] = {
        create_pair(cards[0], cards[4]),
        create_pair(cards[1], cards[5]),
        create_pair(cards[2], cards[6]),
        create_pair(cards[3], cards[7])
    };

    for (u32 i = 0; i < 4; i++) {
        score += pairs[i].score;
    }

    u32 match_index = 0;
    Card_Pair_Match matches[4];
    for (u32 i = 0; i < 4; i++) {
        if (pairs[i].matching) {
            bool8 found_match = false;
            for (u32 j = 0; j < match_index; j++) {
                if (matches[j].number == pairs[i].number) {
                    matches[j].pairs++;
                    found_match = true;
                }
            }
            if (found_match)
                continue;
            matches[match_index++] = { pairs[i].number, 1 };
        }
    }

    for (u32 i = 0; i < match_index; i++) {
        Card_Pair_Match match = matches[i];
        switch(match.pairs) {
            case 2: score -= 10; break;
            case 3: score -= 15; break;
            case 4: score -= 20; break;
        }
    }

    return score;
}

internal void
update_scores(Game *game) {
    for (u32 i = 0; i < game->num_of_players; i++) {
        Player *player = &game->players[i];
        u32 *cards = player->cards;            
        s32 score = get_score(cards);
        //print("Player %d: %d\n", i, score);
        player->scores[game->holes_played] = score;
    }
}

internal u32
get_number_flipped(bool8 *flipped) {
    u32 number_flipped = 0;
    for (u32 i = 0; i < HAND_SIZE; i++) {
        if (flipped[i]) {
            number_flipped++;
        }
    }
    return number_flipped;
}
