struct Ray {
    Vector3 origin;
    Vector3 direction;
};

struct Ray_Intersection {
    u32 number_of_intersections;
    Vector3 point;
    Vector3 normal;
};

struct Triangle {
    Vector3 a, b, c;
};