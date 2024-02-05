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

struct Menu_Button_Style {
    Vector2 dim;
    
    Vector4 default_back_color;
    Vector4 active_back_color;
    Vector4 default_text_color;
    Vector4 active_text_color;
};

struct Menu {
    Menu_Button_Style button_style;
    
    Font *font;

    Vector2 padding;

    Rect rect; // coords and dim of menu
};