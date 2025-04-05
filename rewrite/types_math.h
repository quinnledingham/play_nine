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

inline float32
magnitude(const Vector3 &v)
{
    float32 len_sq = length_squared(v);
    if (len_sq < EPSILON)
        return 0.0f;
    return (float32)sqrt(len_sq);
}

inline float32
distance(const Vector3 v1, const Vector3 v2) {
    float32 x = powf(v2.x - v1.x, 2);
    float32 y = powf(v2.y - v1.y, 2);
    float32 z = powf(v2.z - v1.z, 2);
    return sqrtf(x + y + z);
}

//
// Vector4
//

inline float32 dot_product(const Vector4 l, const Vector4 r) { return (l.x * r.x) + (l.y * r.y) + (l.z * r.z) + (l.w * r.w); }

//
// Quaternion
//

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

inline void operator*=(Quaternion &l, const Quaternion &r) { l = l * r; }

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
m4x4(Pose pose, Vector3 scale) {
    Quaternion x_rot = get_rotation(pose.w, X_AXIS);
    Quaternion y_rot = get_rotation(pose.p, Y_AXIS);
    Quaternion z_rot = get_rotation(pose.k, Z_AXIS);

    //Quaternion rot = z_rot * y_rot * x_rot;
    Quaternion rot = x_rot * z_rot * y_rot;

    return create_transform_m4x4(pose.position, rot, scale);
}

inline Quaternion
rotation(Vector3 orientation) {
    Quaternion x_rot = get_rotation(orientation.w, X_AXIS);
    Quaternion y_rot = get_rotation(orientation.p, Y_AXIS);
    Quaternion z_rot = get_rotation(orientation.k, Z_AXIS);

    Quaternion rot = x_rot * y_rot * z_rot;

    return rot;
}

inline Matrix_4x4 
m4x4(Transform transform) {
    Quaternion x_rot = get_rotation(transform.w, X_AXIS);
    Quaternion y_rot = get_rotation(transform.p, Y_AXIS);
    Quaternion z_rot = get_rotation(transform.k, Z_AXIS);

    //Quaternion rot = z_rot * y_rot * x_rot;
    Quaternion rot = x_rot * z_rot * y_rot;

    return create_transform_m4x4(transform.position, rot, transform.scale);
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

inline Vector4
m4x4_get_row(Matrix_4x4 m, u32 i) {
    Vector4 row;
    row.E[0] = m.E[0][i];
    row.E[1] = m.E[1][i];
    row.E[2] = m.E[2][i];
    row.E[3] = m.E[3][i];
    return row;
}

inline Vector4 
m4x4_mul_v4(Matrix_4x4 m, Vector4 v) {
    
    Vector4 result =  {
        dot_product(m4x4_get_row(m, 0), v),
        dot_product(m4x4_get_row(m, 1), v),
        dot_product(m4x4_get_row(m, 2), v),
        dot_product(m4x4_get_row(m, 3), v)
    };
    return result;
}

inline Matrix_4x4 m4x4_mul_float32(Matrix_4x4 m, float32 f) {
    Matrix_4x4 result = {};
    u32 row = 0;
    u32 column = 0;

    for (u32 i = 0; i < 16; i++) {
        result.E[row][column] = f * m.E[row][column];

        column++;
        if (column == 4) {
            column = 0;
            row++;
        }
    }
    return result;
}

/*
a b
c d
*/

inline float32
determinant_2x2(float32 a, float32 b, float32 c, float32 d) {
    float32 result = (a * d - b * c);
    return result;
}

inline float32
determinant_3x3(Matrix_3x3 m) {
    float32 
    a11 = m.E[0][0], a12 = m.E[0][1], a13 = m.E[0][2],
    a21 = m.E[1][0], a22 = m.E[1][1], a23 = m.E[1][2],
    a31 = m.E[2][0], a32 = m.E[2][1], a33 = m.E[2][2];

    float32 a1 = a11 * determinant_2x2(a22, a23, a32, a33);
    float32 a2 = a12 * determinant_2x2(a21, a23, a31, a33);
    float32 a3 = a13 * determinant_2x2(a21, a22, a31, a32);
    return a1 - a2 + a3;
}

inline Matrix_3x3
get_matrix_3x3(Matrix_4x4 m, u32 row, u32 column) {
    Matrix_3x3 result = {};
    u32 r_row = 0, r_column = 0;

    for (u32 i = 0; i < 4; i++) {
        r_column = 0;
        for (u32 j = 0; j < 4; j++) {
            if (row != i && column != j) {
                result.E[r_row][r_column] = m.E[i][j];
                r_column++;
            }
        }
        if (row != i)
            r_row++;
    }
    
    return result;
}

inline float32
determinant_4x4(Matrix_4x4 m) {
    float32 
    a11 = m.E[0][0], a12 = m.E[0][1], a13 = m.E[0][2], a14 = m.E[0][3],
    a21 = m.E[1][0], a22 = m.E[1][1], a23 = m.E[1][2], a24 = m.E[1][3],
    a31 = m.E[2][0], a32 = m.E[2][1], a33 = m.E[2][2], a34 = m.E[2][3],
    a41 = m.E[3][0], a42 = m.E[3][1], a43 = m.E[3][2], a44 = m.E[3][3];

    float32 a1 = a11 * determinant_3x3(get_matrix_3x3(m, 0, 0));
    float32 a2 = a12 * determinant_3x3(get_matrix_3x3(m, 0, 1));
    float32 a3 = a13 * determinant_3x3(get_matrix_3x3(m, 0, 2));
    float32 a4 = a14 * determinant_3x3(get_matrix_3x3(m, 0, 3));
    return a1 - a2 + a3 - a4;
}

inline Matrix_4x4
m4x4_transpose(Matrix_4x4 m) {
    Matrix_4x4 result = {};
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            result.E[j][i] = m.E[i][j];
        }
    }
    return result;
}

inline Matrix_4x4
adjugate_matrix_4x4(Matrix_4x4 m) {
    Matrix_4x4 result  = {};
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            float32 sign = 1.0f;
            if ((i + j) % 2 != 0)
                sign = -1.0f;

            result.E[i][j] = sign * determinant_3x3(get_matrix_3x3(m, i, j));
        }
    }
    /*
    00 01 02 03
    10 11 12 13
    20 21 22 23
    30 31 32 33
    */
    result = m4x4_transpose(result);

    return result;
}

inline Matrix_4x4
inverse(Matrix_4x4 m) {
    float32 det = determinant_4x4(m);
    Matrix_4x4 adj = adjugate_matrix_4x4(m);
    Matrix_4x4 inverse = m4x4_mul_float32(adj,(1.0f/det));
    return inverse;
}

//
// Colors
//

inline Color_RGB get_color(Vector3 v) { return { u8(v.r), u8(v.g), u8(v.b) }; }
inline Color_RGBA get_color(Vector4 v) { return { u8(v.r), u8(v.g), u8(v.b), u8(v.a) }; }