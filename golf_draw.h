global const Vector4 golf_green        = {  39,  77,  20, 1 };
global const Vector4 golf_yellow       = { 231, 213,  36, 1 };
global const Vector4 golf_light_yellow = { 240, 229, 118, 1 };
global const Vector4 golf_dark_yellow  = { 197, 180,  22, 1 };

/*
(0, 0)
 -> ###############################
    #   #                         #
    # r #            r            #
    # b #                         #
    #   #                         #
    ###############################
    # r #            r            #
    ###############################
*/
struct String_Draw_Info {
    Vector2 dim;      // dim of the text
    Vector2 baseline;

    Vector2 font_dim; // biggest char in font
    Vector2 font_baseline; // baseline for biggest char
};

struct Draw_Context {
  Mesh square;
  Mesh square_3D;
  Mesh rounded_rect;
  Mesh cube;
  Mesh sphere;

  u32 font_id;
};
