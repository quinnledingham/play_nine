//
// Draw_<> structs are meant to be structs to pass arguments for the draw functions.
// Try not use them to store information about a button or textbox. This is because they
// are meant to be shared between many draw calls, changing only what needs to be changed
// between calls (coords).
//

enum GUI_States {
    GUI_DEFAULT,
    GUI_HOVER,
    GUI_PRESSED,
    GUI_ACTIVE
};

struct Draw_Style {
    union {
        struct {
            Vector4 default_back;
            Vector4 default_text;
    
            Vector4 hover_back;
            Vector4 hover_text;

            Vector4 pressed_back;
            Vector4 pressed_text;

            Vector4 active_back; 
            Vector4 active_text;
        };
        Vector4 E[4][2];
    };
};

// hover
// pressed
// active

struct Draw_Button {
    Draw_Style style;
    u32 state;

    Vector2 coords;
    Vector2 dim;

    Font *font;
    const char *text;
};

struct Draw_Textbox {
    Draw_Style style;
    u32 state;

    Vector4 cursor_color;
    u32 cursor_position;
    float32 cursor_width;
    float32 text_shift; // changes depending on cursor position

    Vector2 coords;
    Vector2 dim;

    Font *font;
    const char *text;
};

#define TEXTBOX_SIZE 20

struct Textbox {
    u32 cursor_position;
    float32 shift;
    char text[TEXTBOX_SIZE];
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
    bool8 pressed;
    enum Input_Type active_input_type;

    // Keyboard
    Button up;
    Button down;
    Button left;
    Button right;

    // Because I have to remember what is selected unlike with the mouse where you can
    // always check where the mouse is.
    Vector2_s32 hot;      // what was hot after last round
    Vector2_s32 *hot_ptr; // change this to new hot if needed

    // Mouse
    Vector2_s32 mouse;

    // textbox typing inptu
    s32 *buffer;
    s32 buffer_index;
};

struct Menu {
    Menu_Button_Style button_style;
    Draw_Style style;

    Textbox edit;

    Font *font;
    Vector2 padding;

    Vector2_s32 sections;
    Vector2_s32 interact_region[2]; // sections that can be included

    Vector2_s32 hot_section; // section that is hot
    Vector2_s32 pressed_section;
    Vector2_s32 active_section;

    Vector2_s32 scroll; // sections that are visible in the scroll

    Rect rect; // coords and dim of entire menu

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

