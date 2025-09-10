enum GUI_States {
    GUI_DEFAULT,
    GUI_HOVER,
    GUI_PRESSED,
    GUI_ACTIVE
};

enum GUI_Align {
    ALIGN_CENTER,
    ALIGN_LEFT,    
    ALIGN_RIGHT,
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
  u32 index; // what GUI index is active
  void *value;
  u32 value_type;
  u32 cursor_position;
  float32 shift;
  char text[TEXTBOX_SIZE];

};

struct GUI {
  u32 index;
  u32 hover;
  u32 pressed;
  u32 active;

  Textbox edit;
  Draw_Style style; // for components

  Vector4 background_color; // clears the windows to this color
  Vector4 back_color; // color of just the part of the screen covered by gui with dim

  Vector2 dim; // %
  Vector2_s32 segments; // splits of dim

  Vector2 shift; // %
  Vector2 coords; // px
  Vector2 segment_dim; // px

  Vector2 padding; // %
  Vector2 padding_px;

  Vector2 backdrop;
  Vector2 backdrop_px;

  s32 (*draw)(GUI *gui);

  void start() { 
    index = 1; 
  }

  void end() {
    if (hover > index - 1)
      hover = index - 1;
  }

  void close() {
    hover = 0;
    pressed = 0;
    active = 0;
    edit.index = 0;
  }
};

enum GUI_Ids {
  GUI_MAIN_MENU,
  GUI_TEST,
  GUI_PAUSE,
  GUI_DEBUG,

  GUI_COUNT
};

struct GUI_Manager {
  GUI guis[GUI_COUNT];

  Stack<u32> indices = Stack<u32>(10); // index at the top is the gui that is currently rendered
};

