struct Controller {
    union {
        struct {
            // Game Controls

            Button one;
            Button two;
            Button three;
            Button four;
            
            Button five;
            Button six;
            Button seven;
            Button eight;

            Button nine;
            Button zero;

            // Camera Controls
            Button camera_toggle;

            Button forward;
            Button backward;
            Button left;
            Button right;
            Button up;
            Button down;

            Button pause;
            Button select;

            // Mouse Controls
            Button mouse_left;
        };
        Button buttons[20];
    };
};

struct Input {
    Controller controller;
    Vector2_s32 mouse;
    enum Input_Type active;
};