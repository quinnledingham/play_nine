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

struct Game {
	u32 pile[DECK_SIZE];
	u32 top_of_pile;
};