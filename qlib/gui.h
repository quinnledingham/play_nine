//
// Draw_<> structs are meant to be structs to pass arguments for the draw functions.
// Try not use them to store information about a button or textbox. This is because they
// are meant to be shared between many draw calls, changing only what needs to be changed
// between calls (coords).
//

struct Draw_Button {
    Vector4 default_back_color;
    Vector4 default_text_color;

    Vector4 active_back_color;
    Vector4 active_text_color;

    bool8 active;

    Vector2 coords;
    Vector2 dim;

    Font *font;
    const char *text;
};

struct Draw_Textbox {
    Vector4 back_color;
    Vector4 text_color;

    Vector2 coords;
    Vector2 dim;

    float32 text_shift; // changes depending on cursor position

    Font *font;
    const char *text;
};

struct Textbox {
    u32 cursor_position;
    char text[20];
};

//
// Menu
//

struct Menu_Button_Style {
    Vector2 dim;
    
    Vector4 default_back_color;
    Vector4 active_back_color;
    Vector4 default_text_color;
    Vector4 active_text_color;
};

struct Menu_Input {
    // Both
    bool8 select;
    enum Input_Type active_input_type;

    // Keyboard
    Button up;
    Button down;
    Button left;
    Button right;

    // Because I have to remember what is selected unlike with the mouse where you can
    // always check where the mouse is.
    Vector2_s32 active;      // what was active after last round
    Vector2_s32 *active_ptr; // change this to new active if needed

    // Mouse
    Vector2_s32 mouse;
};

struct Menu {
    Menu_Button_Style button_style;
    Font *font;
    Vector2 padding;

    Vector2_s32 sections;
    Vector2_s32 hot[2]; // sections that can be included
    Vector2_s32 active_section;
    Rect rect; // coords and dim of entire menu

    s32 active;
    bool8 initialized;
};

//
// Misc
//

struct Onscreen_Notifications {
    static const u32 number_of_lines = 10;
    char memory[number_of_lines][90]; // number of lines of memory used
    u32 lines;
    float32 times[number_of_lines]; // how long left on displaying the line
    Vector4 colors[number_of_lines]; // the color for each line

    Font *font;
    Vector4 text_color;
};

