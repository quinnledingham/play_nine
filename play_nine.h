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
			Button forward;
			Button backward;
			Button left;
			Button right;
			Button up;
			Button down;

            Button pause;
        };
        Button buttons[7];
    };
};


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

	Controller controller = {};

	Render_Pipeline basic_pipeline;
	Scene scene;
	Scene ortho_scene;
	Descriptor_Set *scene_set;
	Descriptor_Set *scene_ortho_set;
	Camera camera;
	Mesh rect;

	Assets assets;
};