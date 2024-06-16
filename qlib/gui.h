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
            Vector4 background_color;
            Vector4 background_color_hover;
            Vector4 background_color_pressed;
            Vector4 background_color_active; // mostly for textboxs right now
        };
        Vector4 background_colors[4];
    };

    union {
        struct {
            Vector4 text_color;
            Vector4 text_color_hover;
            Vector4 text_color_pressed;
            Vector4 text_color_active;
        };
        Vector4 text_colors[4];
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
    u32 text_align;
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
    u32 text_align; // inside textbox

    const char *label;
    Vector4 label_color;
};

#define TEXTBOX_SIZE 20

struct Textbox {
    u32 cursor_position;
    float32 shift;
    char text[TEXTBOX_SIZE];
    u32 index; // what GUI index is active
};

//
// GUI
//

struct GUI_Input {
    enum Input_Type *active_input_type;

    // Controller Input
    Button *select;
    Button *left;
    Button *up;
    Button *right;
    Button *down;

    // Mouse Input
    Vector2_s32 *mouse;
    Button *mouse_left;

    // Keyboard Input (Textbox)
    s32 *buffer;
    s32 *buffer_index;  
};

enum GUI_Align {
    ALIGN_CENTER,
    ALIGN_LEFT,    
    ALIGN_RIGHT,
};

struct GUI {
    u32 index; // = 1, starts at 1, reset to 1 every frame, because hover/pressed/active default is 0
    u32 hover;
    u32 pressed;
    u32 active;
    GUI_Input input;

    Textbox edit;

    Rect rect;

    Font *font;
    u32 text_align;
    Draw_Style style;

    // dropdown
    bool8 enabled; // when true in gui_update the check if something else is active is disabled

    bool8 close_at_end;

    // Audio
    Audio hovered;
    Audio activated;

    void start() { 
        index = 1; 
    };

    void close() {
        hover = 0;
        pressed = 0;
        active = 0;
        edit.index = 0;
    }
    
/*
    bool8 is_hover(Rect rect) {
        bool8 hover = false;
        
        if (input.active_input_type == KEYBOARD_INPUT) {

            if (gui->hover == 0)
                gui->hover = 1;
        } else if (input.active_input_type == MOUSE_INPUT) {
            gui_select = *gui->input.mouse_left;

            if (coords_in_rect(*gui->input.mouse, coords, dim)) {
                gui->hover = gui->index;
            } else if (gui->hover == gui->index) {
                gui->hover = 0;
            }
        }

        return hover;
    }
    */
};

GUI gui = {};

//
// Menu
//

struct Menu {    
    Vector2_s32 sections;
    Vector2_s32 interact_region[2]; // sections that can be included
    
    Vector2_s32 hover_section; // saves where the controller is hovering
    Vector2_s32 hover_section_updated;

    bool8 button_confirm_active;

    GUI gui;

    void start() {
        if (hover_section.y < interact_region[0].y)
            hover_section_updated = interact_region[0];
        else
            hover_section_updated = hover_section;


        gui.start();
    }

    void end() {
        hover_section = hover_section_updated;

        if (gui.close_at_end) {
            button_confirm_active = false;
            gui.close_at_end = false;
            hover_section = interact_region[0];
            gui.close();
        }
    }
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

