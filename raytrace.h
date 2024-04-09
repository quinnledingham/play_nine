struct Ray {
    Vector3 origin;
    Vector3 direction;
};

struct Ray_v4 {
    Vector4 origin;
    Vector4 direction;
};

struct Ray_Intersection {
    Vector4 point;
    Vector4 normal;
    s32 number_of_intersections;
};

struct Triangle {
    Vector3 a, b, c;
};

struct Triangle_v4 {
    Vector4 a, b, c;
};