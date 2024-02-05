#ifndef SHAPES_H
#define SHAPES_H

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

#endif // SHAPES_H
