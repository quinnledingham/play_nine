//
// Drawing
//

struct Global_Shader {
    Vector4 resolution; // rect
    Vector4 time; // time, time_delta, frame_rate
};

struct Scene {
    Matrix_4x4 view;
    Matrix_4x4 projection;
};

struct Texture_Region {
    Vector2 uv_offset;
    Vector2 uv_scale;
};

struct Local {
    Vector4 text;
    Vector4 color;
    Texture_Region region;
};

struct Object {
    Matrix_4x4 model;
    s32 index;
};

struct Camera {
    // defines camera
    union {
        struct {
            Vector3 position;
            union {
                struct {
                    float32 roll, pitch, yaw;
                };
                Vector3 orientation;
            };
        };
        Pose pose;
    };

    // initialized by pose
    Vector3 right;
    Vector3 up;
    Vector3 direction;

    float32 fov;

    //Animation *animation;
};

