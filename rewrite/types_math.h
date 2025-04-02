inline s32
clamp(s32 value, s32 low, s32 high) {
    if (value < low)       return low;
    else if (value > high) return high;
    else                   return value;
}

inline void
clamp(float32 *value, float32 low, float32 high) {
    if (*value < low)
        *value = low;
    if (*value > high)
        *value = high;
}

//
// Vector2
//

inline Vector2 operator+(const Vector2 &l, const Vector2 &r) { return { l.x + r.x, l.y + r.y }; }
inline Vector2 operator+(const Vector2 &l, const float32 &r) { return { l.x + r  , l.y + r   }; }
inline Vector2 operator-(const Vector2 &l, const Vector2 &r) { return { l.x - r.x, l.y - r.y }; }
inline Vector2 operator*(const Vector2 &l, const Vector2 &r) { return { l.x * r.x, l.y * r.y }; }
inline Vector2 operator*(const Vector2 &l, const float32 &r) { return { l.x * r  , l.y * r   }; }
inline Vector2 operator/(const Vector2 &l, const Vector2 &r) { return { l.x / r.x, l.y / r.y }; }
inline Vector2 operator/(const Vector2 &l, const float32 &r) { return { l.x / r  , l.y / r   }; }
inline Vector2 operator-(const Vector2 &v)                   { return { -v.x     , -v.y      }; }

inline void operator+=(Vector2 &l, const Vector2 &r) { l.x = l.x + r.x; l.y = l.y + r.y; }
inline void operator-=(Vector2 &l, const Vector2 &r) { l.x = l.x - r.x; l.y = l.y - r.y; }
inline void operator-=(Vector2 &l, const float32 &r) { l.x = l.x - r;   l.y = l.y - r;   }
inline void operator*=(Vector2 &l, const float32 &r) { l.x = l.x * r;   l.y = l.y * r;   }
inline void operator/=(Vector2 &l, const Vector2 &r) { l.x = l.x / r.x; l.y = l.y / r.y; }
inline void operator/=(Vector2 &l, const float32 &r) { l.x = l.x / r;   l.y = l.y / r;   }
inline bool operator==(Vector2 &l, const float32 &r) { return (l.x == r && l.y == r);    }

inline Vector2_s32 cv2(Vector2 v) { return Vector2_s32{ (s32)v.x, (s32)v.y }; }

//
// Vector2_s32 
//

inline Vector2_s32 operator+(const Vector2_s32 &l, const Vector2_s32 &r) { return { l.x + r.x, l.y + r.y }; }
inline Vector2_s32 operator+(const Vector2_s32 &l, const s32 &r) { return { l.x + r,   l.y + r   }; }
inline Vector2_s32 operator-(const Vector2_s32 &l, const Vector2_s32 &r) { return { l.x - r.x, l.y - r.y }; }
inline Vector2_s32 operator-(const Vector2_s32 &l, const int &r) { return { l.x - r,   l.y - r   }; }
inline Vector2_s32 operator*(const Vector2_s32 &l, const s32 &r) { return { l.x * r,   l.y * r   }; }
inline Vector2_s32 operator/(const Vector2_s32 &l, const s32 &r) { return { l.x / r  , l.y / r   }; }

inline void operator+=(Vector2_s32 &l, const Vector2_s32 &r) { l.x = l.x + r.x; l.y = l.y + r.y; }
inline void operator+=(Vector2_s32 &l, const s32 &r) { l.x = l.x + r;   l.y = l.y + r;   }
inline void operator-=(Vector2_s32 &l, const Vector2_s32 &r) { l.x = l.x - r.x; l.y = l.y - r.y; }
inline void operator*=(Vector2_s32 &l, const s32 &r) { l.x = l.x * r;   l.y = l.y * r;   }
inline bool operator==(const Vector2_s32 &l, const Vector2_s32 &r) { if (l.x == r.x && l.y == r.y) return true; return false; }
inline bool operator!=(const Vector2_s32 &l, const Vector2_s32 &r) { if (l.x != r.x || l.y != r.y) return true; return false; }
inline bool operator==(Vector2_s32 &l, const float32 &r) { return l.x == r && l.y == r;      }

inline Vector2 cv2(Vector2_s32 v) { return { (float32)v.x, (float32)v.y }; }

//
// Vector3
//

inline Vector3 operator+(const Vector3 &l, const Vector3  &r) { return { l.x + r.x, l.y + r.y, l.z + r.z }; }
inline Vector3 operator-(const Vector3 &l, const Vector3  &r) { return { l.x - r.x, l.y - r.y, l.z - r.z }; }
inline Vector3 operator*(const Vector3 &l, const Vector3  &r) { return { l.x * r.x, l.y * r.y, l.z * r.z }; }
inline Vector3 operator*(const Vector3 &l, float      r) { return {l.x * r,    l.y * r,   l.z * r   }; }
inline Vector3 operator/(const Vector3 &l, const Vector3  &r) { return { l.x / r.x, l.y / r.y, l.z / r.z }; }
inline Vector3 operator/(const Vector3 &l, const float32 &r) { return { l.x / r,   l.y / r,   l.z / r   }; }
inline Vector3 operator-(const Vector3 &v)               { return {-v.x    ,  -v.y    ,  -v.z       }; }

inline bool operator==(const Vector3 &a, const Vector3 &b) { if (a.x == b.x && a.y == b.y && a.z == b.z)   return true; return false; }
inline bool operator==(const Vector3 &v, const float f)          { if (v.x == f   && v.y == f   && v.z == f)   return true; return false; }
inline void operator+=(Vector3 &l, const Vector3 &r) { l.x = l.x + r.x; l.y = l.y + r.y; l.z = l.z + r.z; }
inline void operator+=(Vector3 &l, const float32 &r) { l.x = l.x + r;   l.y = l.y + r;   l.z = l.z + r;   }
inline void operator-=(Vector3 &l, const Vector3 &r) { l.x = l.x - r.x; l.y = l.y - r.y; l.z = l.z - r.z; }
inline void operator-=(Vector3 &l, const float32 &r) { l.x = l.x - r;   l.y = l.y - r;   l.z = l.z - r;   }

inline float32 dot_product(const Vector3 &l, const Vector3 &r) { return (l.x * r.x) + (l.y * r.y) + (l.z * r.z); }
inline float32 length_squared(const Vector3 &v) { return (v.x * v.x) + (v.y * v.y) + (v.z * v.z); }

inline Vector4 cv4(Vector3 v) { return { v.x, v.y, v.z, 0 }; };

inline void
normalize(Vector3 &v)
{
    float32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return;
    float32 inverse_length = 1.0f / sqrtf(len_sq);
    v.x *= inverse_length;
    v.y *= inverse_length;
    v.z *= inverse_length;
}

inline Vector3
normalized(const Vector3 &v)
{
    float32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return v;
    float32 inverse_length = 1.0f / sqrtf(len_sq);
    return { v.x * inverse_length, v.y * inverse_length, v.z * inverse_length };
}

inline Vector3
cross_product(const Vector3 &l, const Vector3 &r)
{
    return 
    {
        (l.y * r.z - l.z * r.y),
        (l.z * r.x - l.x * r.z),
        (l.x * r.y - l.y * r.x)
    };
}

// Quaternion

inline float32 length_squared(const Quaternion &v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }

inline Quaternion 
operator*(const Quaternion &l, const Quaternion &r)  {
    return {
         r.x * l.w + r.y * l.z - r.z * l.y + r.w * l.x,
        -r.x * l.z + r.y * l.w + r.z * l.x + r.w * l.y,
         r.x * l.y - r.y * l.x + r.z * l.w + r.w * l.z,
        -r.x * l.x - r.y * l.y - r.z * l.z + r.w * l.w
    };
}

inline Vector3 
operator*(const Quaternion& q, const Vector3& v) {
    return q.vector * 2.0f * dot_product(q.vector, v) + v * 
           (q.scalar * q.scalar - dot_product(q.vector, q.vector)) + 
           cross_product(q.vector, v) * 2.0f * q.scalar;
}

inline Quaternion
normalized(const Quaternion &v) {
    float32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return { 0, 0, 0, 1 };
    float32 inverse_length = 1.0f / sqrtf(len_sq);
    return {v.x * inverse_length, v.y * inverse_length, v.z * inverse_length, v.w * inverse_length};
}

inline Quaternion 
get_rotation(float32 angle, const Vector3& axis) {
    Vector3 norm = normalized(axis);
    float32 s = sinf(angle * 0.5f);
    return { norm.x * s, norm.y * s, norm.z * s, cosf(angle * 0.5f) };
}

/*
    Matrix_4x4
*/

inline Matrix_4x4
get_frustum(float32 l, float32 r, float32 b, float32 t, float32 n, float32 f) {
    if (l == r || t == b || n == f) {
        log_error("get_frustum(): Invalid frustum\n");
        return {};
    }
    
    return {
        (2.0f * n) / (r - l), 0, 0, 0,
        0, (2.0f * n) / (t - b), 0, 0,
        (r + l) / (r - l), (t + b) / (t - b), (-(f + n)) / (f - n), -1,
        0, 0, (-2 * f * n) / (f - n), 0
    };
}

inline Matrix_4x4
perspective_projection(float32 fov, float32 aspect_ratio, float32 n, float32 f) {
    float32 y_max = n * tanf(fov * PI / 360.0f);
    float32 x_max = y_max * aspect_ratio;
    return get_frustum(-x_max, x_max, -y_max, y_max, n, f);
}

inline Matrix_4x4
orthographic_projection(float32 l, float32 r, float32 b, float32 t, float32 n, float32 f) {
    if (l == r || t == b || n == f) {
        log_error("orthographic_projection(): Invalid arguments\n");
        return {};
    }
    return {
        2.0f / (r - l),  0,              0,              0,
        0,               2.0f / (t - b), 0,              0,
        0,               0,             -2.0f / (f - n), 0,
        -((r+l)/(r-l)), -((t+b)/(t-b)), -((f+n)/(f-n)),  1
    };
}

inline Matrix_4x4
identity_m4x4() {
    return {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

inline Matrix_4x4 
create_transform_m4x4(Vector3 position, Quaternion rotation, Vector3 scale) {
    Vector3 x = {1, 0, 0};
    Vector3 y = {0, 1, 0};
    Vector3 z = {0, 0, 1};
    
    x = rotation * x;
    y = rotation * y;
    z = rotation * z;
    
    x = x * scale.x;
    y = y * scale.y;
    z = z * scale.z;
    
    return {
        x.x, x.y, x.z, 0,
        y.x, y.y, y.z, 0,
        z.x, z.y, z.z, 0,
        position.x, position.y, position.z, 1
    };
}

inline Matrix_4x4 
create_transform_m4x4(Vector2 position, Vector2 scale) {
    Vector3 x = {1, 0, 0};
    Vector3 y = {0, 1, 0};
    Vector3 z = {0, 0, 1};
    
    x = x * scale.x;
    y = y * scale.y;

    return {
        x.x, x.y, x.z, 0,
        y.x, y.y, y.z, 0,
          0,   0,   1, 0,
        position.x, position.y, 0, 1
    };
}

inline Matrix_4x4
look_at(const Vector3 &position, const Vector3 &target, const Vector3 &up) {
    Vector3 f = normalized(target - position) * -1.0f;
    Vector3 r = cross_product(up, f);
    if (r == 0) 
        return identity_m4x4();
    normalize(r);
    Vector3 u = normalized(cross_product(f, r));
    Vector3 t = { -dot_product(r, position), -dot_product(u, position), -dot_product(f, position) };
    
    return {
        r.x, u.x, f.x, 0,
        r.y, u.y, f.y, 0,
        r.z, u.z, f.z, 0,
        t.x, t.y, t.z, 1
    };
}

//
// Colors
//

inline Color_RGB get_color(Vector3 v) { return { u8(v.r), u8(v.g), u8(v.b) }; }
inline Color_RGBA get_color(Vector4 v) { return { u8(v.r), u8(v.g), u8(v.b), u8(v.a) }; }