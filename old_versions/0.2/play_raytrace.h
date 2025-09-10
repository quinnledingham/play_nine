struct alignas(16) Ray {
    Vector3 origin;
    Vector3 direction;
};

struct alignas(16) Ray_Intersection {
    Vector3 point;
    Vector3 normal;
    s32 number_of_intersections;
};

struct alignas(16) Triangle {
  Vector3 a;
  Vector3 b;
  Vector3 c;
};