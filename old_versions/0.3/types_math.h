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
inline float32 length_squared(const Vector2 &v) { return (v.x * v.x) + (v.y * v.y); }

inline float32
magnitude(const Vector2 &v) {
    float32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return 1;
    return (float32)sqrt(len_sq);
}

inline float32
distance(const Vector2 v1, const Vector2 v2) {
    float32 x = powf(v2.x - v1.x, 2);
    float32 y = powf(v2.y - v1.y, 2);
    return sqrtf(x + y);
}

inline void
normalize(Vector2 &v) {
    float32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return;
    float32 inverse_length = 1.0f / sqrtf(len_sq);
    v = { v.x * inverse_length, v.y * inverse_length };
}

inline Vector2
normalized(const Vector2 &v) {
    float32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return v;
    float32 inverse_length = 1.0f / sqrtf(len_sq);
    return { v.x * inverse_length, v.y * inverse_length };
}

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
// Vector2_u32
//

inline Vector2 cv2(Vector2_u32 v) { return { (float32)v.x, (float32)v.y }; }


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


//
// Quaternion
//

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

//
// Matrix
//

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
transpose(Matrix_4x4 m) {
    Matrix_4x4 result = {};
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            result.E[j][i] = m.E[i][j];
        }
    }
    return result;
}

inline Matrix_4x4
perspective_projection2(float32 fov, float32 aspect_ratio, float32 n, float32 f) {
    float32 tan_fov = tanf(fov / 2.0f);
    Matrix_4x4 P = {
        aspect_ratio/tan_fov, 0, 0, 0,
        0, 1/tan_fov, 0, 0,
        0, 0, f/(f-n), -(n*f)/(f-n),
        0, 0, 1, 0
    };
    return transpose(P);
}

inline Matrix_4x4
orthographic_projection(float32 l, float32 r, float32 b, float32 t, float32 n, float32 f) {
    if (l == r || t == b || n == f) {
        app_log_error("orthographic_projection(): Invalid arguments\n");
        return {};
    }
    return {
        2.0f / (r - l),  0,              0,              0,
        0,               2.0f / (t - b), 0,              0,
        0,               0,             -2.0f / (f - n), 0,
        -((r+l)/(r-l)), -((t+b)/(t-b)), -((f+n)/(f-n)),  1
    };
}