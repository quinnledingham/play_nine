inline s32
clamp(s32 value, s32 low, s32 high) {
    if (value < low)       return low;
    else if (value > high) return high;
    else                   return value;
}

internal void
clamp(float32 *value, float32 low, float32 high) {
    if (*value < low)
        *value = low;
    if (*value > high)
        *value = high;
}

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

inline float32 dot_product(const Vector3 &l, const Vector3 &r) { return (l.x * r.x) + (l.y * r.y) + (l.z * r.z); }
inline float32 length_squared(const Vector3 &v) { return (v.x * v.x) + (v.y * v.y) + (v.z * v.z); }

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

