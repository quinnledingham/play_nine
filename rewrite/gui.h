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

#define TEXTBOX_SIZE 20

struct Textbox {
    u32 index; // what GUI index is active
    u32 cursor_position;
    float32 shift;
    char text[TEXTBOX_SIZE];
};

struct GUI_Draw_Command {

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