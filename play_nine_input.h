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

            Button pile;
            Button discard;

            // Camera Controls
            Button camera_toggle;
            Button save_camera;

            Button forward;
            Button backward;
            Button left;
            Button right;
            Button up;
            Button down;

            Button pause;
            Button select;
            Button pass;

            // Mouse Controls
            Button mouse_left;
        };
        Button buttons[22];
    };
};
