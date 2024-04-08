#ifndef SHAPES_H
#define SHAPES_H

Render_Pipeline basic_pipeline;
Render_Pipeline color_pipeline;
Compute_Pipeline ray_pipeline;

//
// shapes.h contains the functions for 2D rendering
//

struct Rect {
    Vector2 coords;
    Vector2 dim;
    float32 rotation; // radians
};

internal Vector2
get_centered(Rect in, Rect out) {
    return { 
        (out.dim.x/2.0f) - (in.dim.x / 2.0f), 
        (out.dim.y/2.0f) - (in.dim.y / 2.0f)
    };
}

internal Rect
get_centered_rect(Rect og, float32 x_percent, float32 y_percent) {
    Rect rect = {};
    rect.dim.x = og.dim.x * x_percent;
    rect.dim.y = og.dim.y * y_percent;
    rect.coords = get_centered(rect, og);
    rect.coords += og.coords;
    return rect;
}

internal bool8
coords_in_rect(Vector2_s32 in, Vector2 coords, Vector2 dim) {
    if (in.x >= coords.x && in.x <= coords.x + dim.x &&
        in.y >= coords.y && in.y <= coords.y + dim.y)
        return true;
    return false;
}

#endif // SHAPES_H
