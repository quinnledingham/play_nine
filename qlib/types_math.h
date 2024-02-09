#ifndef TYPES_MATH_H
#define TYPES_MATH_H

//
// Vector2
//

inline Vector2 operator+(const Vector2 &l, const Vector2  &r) { return { l.x + r.x, l.y + r.y }; }
inline Vector2 operator+(const Vector2 &l, const float32 &r)  { return { l.x + r  , l.y + r   }; }
inline Vector2 operator-(const Vector2 &l, const Vector2  &r) { return { l.x - r.x, l.y - r.y }; }
inline Vector2 operator*(const Vector2 &l, const Vector2  &r) { return { l.x * r.x, l.y * r.y }; }
inline Vector2 operator*(const Vector2 &l, const float32 &r)  { return { l.x * r  , l.y * r   }; }
inline Vector2 operator/(const Vector2 &l, const Vector2  &r) { return { l.x / r.x, l.y / r.y }; }
inline Vector2 operator/(const Vector2 &l, const float32 &r)  { return { l.x / r  , l.y / r   }; }
inline Vector2 operator-(const Vector2 &v)                    { return { -v.x     , -v.y      }; }

inline void operator+=(Vector2 &l, const Vector2  &r) { l.x = l.x + r.x; l.y = l.y + r.y; }
inline void operator-=(Vector2 &l, const Vector2  &r) { l.x = l.x - r.x; l.y = l.y - r.y; }
inline void operator-=(Vector2 &l, const float32 &r)  { l.x = l.x - r;   l.y = l.y - r;   }
inline void operator*=(Vector2 &l, const float32 &r)  { l.x = l.x * r;   l.y = l.y * r;   }
inline void operator/=(Vector2 &l, const Vector2  &r) { l.x = l.x / r.x; l.y = l.y / r.y; }
inline void operator/=(Vector2 &l, const float32 &r)  { l.x = l.x / r;   l.y = l.y / r;   }

inline float32 dot_product(const Vector2 &l, const Vector2 &r) { return (l.x * r.x) + (l.y * r.y); }
inline float32 length_squared(const Vector2 &v) { return (v.x * v.x) + (v.y * v.y); }
inline void print(const Vector2 &v) { print("Vector2: %f, %f", v.x, v.y); }
inline Vector2_s32 cv2(Vector2 v) { return Vector2_s32{ (s32)v.x, (s32)v.y }; }

inline Vector2 v2_add(const Vector2 l, const Vector2 r) {
    Vector2 result = { l.x + r.x, l.y + r.y };
    return result;
}

inline 
Vector2 pow(const Vector2 &v, u32 exponent)
{
    Vector2 result = v;
    for (u32 i = 1; i < exponent; i++) { result.x *= v.x; result.y *= v.y; }
    return result;
}

inline Vector2
normalized(const Vector2 &v)
{
    float32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return v;
    float32 inverse_length = 1.0f / sqrtf(len_sq);
    return { v.x * inverse_length, v.y * inverse_length };
}

inline Vector2
projection_onto_line(Vector2 v, Vector2 line)
{
    return line * (dot_product(v, line) / dot_product(line, line));
}

inline float32
magnitude(const Vector2 &v)
{
    float32 len_sq = length_squared(v);
    if (len_sq < EPSILON) return 1;
    return (float32)sqrt(len_sq);
}

inline float32
angle_between(const Vector2 &a, const Vector2 &b)
{
    float32 dot = (dot_product(a, b));
    float32 cos_theta = dot / (magnitude(a) * magnitude(b));
    
    if      (cos_theta >  1.0) cos_theta =  1.0;
    else if (cos_theta < -1.0) cos_theta = -1.0;
    
    float32 result = acosf(cos_theta);
    return result;
}

//
// Vector2_s32 
//

inline Vector2_s32 operator+(const Vector2_s32 &l, const Vector2_s32 &r) { return { l.x + r.x, l.y + r.y }; }
inline Vector2_s32 operator+(const Vector2_s32 &l, const s32 &r) { return { l.x + r,   l.y + r   }; }
inline Vector2_s32 operator-(const Vector2_s32 &l, const Vector2_s32 &r) { return { l.x - r.x, l.y - r.y }; }
inline Vector2_s32 operator-(const Vector2_s32 &l, const int &r) { return { l.x - r,   l.y - r   }; }
inline Vector2_s32 operator*(const Vector2_s32 &l, const s32 &r) { return { l.x * r,   l.y * r   }; }

inline void operator+=(Vector2_s32 &l, const Vector2_s32 &r) { l.x = l.x + r.x; l.y = l.y + r.y; }
inline void operator+=(Vector2_s32 &l, const s32 &r) { l.x = l.x + r;   l.y = l.y + r;   }
inline void operator*=(Vector2_s32 &l, const s32 &r) { l.x = l.x * r;   l.y = l.y * r;   }
inline bool operator==(const Vector2_s32 &l, const Vector2_s32 &r) { if (l.x == r.x && l.y == r.y) return true; return false; }
inline bool operator!=(const Vector2_s32 &l, const Vector2_s32 &r) { if (l.x != r.x || l.y != r.y) return true; return false; }

inline Vector2 cv2(Vector2_s32 v) { return { (float32)v.x, (float32)v.y }; }
inline void print(const Vector2_s32 &v) { print("Vector2_s32: %d, %d", v.x, v.y); }

inline Vector2_s32
normalized(const Vector2_s32 &v)
{
    Vector2_s32 n = {};
    
    if      (v.x >  0) n.x =  1;
    else if (v.x == 0) n.x =  0;
    else if (v.x <  0) n.x = -1;
    
    if      (v.y >  0) n.y =  1;
    else if (v.y == 0) n.y =  0;
    else if (v.y <  0) n.y = -1;
    
    return n;
}

inline s32
clamp(s32 value, s32 low, s32 high) {
    if (value < low)       return low;
    else if (value > high) return high;
    else                   return value;
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

inline void operator+=(Vector3 &l, const Vector3 &r)  { l.x = l.x + r.x; l.y = l.y + r.y; l.z = l.z + r.z; }
inline void operator+=(Vector3 &l, const float32 &r) { l.x = l.x + r;   l.y = l.y + r;   l.z = l.z + r;   }
inline void operator-=(Vector3 &l, const Vector3 &r)  { l.x = l.x - r.x; l.y = l.y - r.y; l.z = l.z - r.z; }
inline void operator-=(Vector3 &l, const float32 &r) { l.x = l.x - r;   l.y = l.y - r;   l.z = l.z - r;   }
inline void operator*=(Vector3 &l, Vector3 &r)        { l.x *= r.x;      l.y *= r.y;      l.z *= r.z;      }
inline bool operator==(const Vector3 &l, const Vector3 &r) { if (l.x == r.x && l.y == r.y && l.z == r.z) return true; return false; }
inline bool operator==(const Vector3 &v, float f)     { if (v.x == f   && v.y == f   && v.z == f)   return true; return false; }

inline float32 dot_product(const Vector3 &l, const Vector3 &r) { return (l.x * r.x) + (l.y * r.y) + (l.z * r.z); }
inline float32 length_squared(const Vector3 &v) { return (v.x * v.x) + (v.y * v.y) + (v.z * v.z); }

inline Vector3 
power(const Vector3 &v, u32 exponent)
{
    Vector3 result = v;
    for (u32 i = 1; i < exponent; i++) { 
        result.x *= v.x; 
        result.y *= v.y;
        result.z *= v.z;
    }
    return result;
}

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
angle_between(const Vector3 &a, const Vector3 &b)
{
    float32 numerator = dot_product(a, b);
    float32 denominator = (magnitude(a) * magnitude(b));
    
    if (denominator == 0.0f)
        return 0.0f;

    float32 input = numerator / denominator;

    if (input > 1.0f)
        input = 1.0f;
    else if (input < -1.0f)
        input = -1.0f;

    return acosf(input);
}

inline Vector3
projection_onto_line(Vector3 v, Vector3 line)
{
    return line * (dot_product(v, line) / dot_product(line, line));
}

inline Vector3
average(const Vector3 &l, const Vector3 &r) {
    Vector3 result = {};
    result.x = (l.x + r.x) / 2.0f;
    result.y = (l.y + r.y) / 2.0f;
    result.z = (l.x + r.z) / 2.0f;
    return result;
}

inline void
print(const Vector3 v) {
    print("%f %f %f", v.x, v.y, v.z);
}

//
// Vector4
//

inline Vector4 operator*(const Vector4 &l, const Vector4 &r) { return { l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w }; }
inline Vector4 operator*(const Vector4 &l, float     r) { return { l.x * r, l.y * r, l.z * r, l.w * r }; }
inline float32 length_squared(const Vector4 &v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }
inline bool operator==(const Vector4 &l, const Vector4 &r) { if (l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w) return true; return false; }

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

// Returns a Quaternion which contains the rotation between two vectors.
// The two vectors are treated like they are points in the same sphere.
internal Quaternion
from_to(const Vector3& from, const Vector3& to) {
    Vector3 f = normalized(from);
    Vector3 t = normalized(to);
    if (f == t)
    {
        return { 0, 0, 0, 1 };
    }
    else if (f == t * -1.0f)
    {
        Vector3 ortho = { 1, 0, 0 };
        if (fabsf(f.y) < fabsf(f.x))
            ortho = { 0, 1, 0 };
        if (fabsf(f.z) < fabs(f.y) && fabs(f.z) < fabsf(f.x))
            ortho = { 0, 0, 1 };
        Vector3 axis = normalized(cross_product(f, ortho));
        return { axis.x, axis.y, axis.z, 0.0f };
    }
    Vector3 half = normalized(f + t);
    Vector3 axis = cross_product(f, half);
    return { axis.x, axis.y, axis.z, dot_product(f, half) };
}

inline Quaternion 
get_rotation_to_direction(const Vector3& direction, const Vector3& up) {
    // Find orthonormal basis vectors
    Vector3 forward = normalized(direction);
    Vector3 norm_up = normalized(up);
    Vector3 right = cross_product(norm_up, forward);
    norm_up = cross_product(forward, right);
    
    Quaternion world_to_object = from_to({ 0, 0, 1 }, forward); // From world forward to object forward
    Vector3 object_up = { 0, 1, 0 };
    object_up = world_to_object * object_up;   // What direction is the new object up?
    Quaternion u_to_u = from_to(object_up, norm_up); // From object up to desired up
    Quaternion result = world_to_object * u_to_u;    // Rotate to forward direction then twist to correct up
    
    return normalized(result);
}

//
// Matrix_4x4
//

inline void
print_m4x4(Matrix_4x4 matrix) {
    for (int i = 0; i < 16; i++) {
        s32 row = i / 4;
        s32 column = i - (row * 4);
        printf("%f ", matrix.E[row][column]);
        if ((i + 1) % 4 == 0)
            printf("\n");
    }
}

inline Matrix_4x4
get_frustum(float32 l, float32 r, float32 b, float32 t, float32 n, float32 f) {
    if (l == r || t == b || n == f) {
        logprint("get_frustum", "Invalid frustum");
        return {};
    }
    
    return {
        (2.0f * n) / (r - l), 0, 0, 0,
        0, (2.0f * n) / (t - b), 0, 0,
        (r + l) / (r - l), (t + b) / (t - b), (-(f + n)) / (f - n), -1,
        0, 0, (-2 * f * n) / (f - n), 0
    };
}

// Only difference is the y_max's
#ifdef OPENGL

inline Matrix_4x4
perspective_projection(float32 fov, float32 aspect_ratio, float32 n, float32 f)
{
    // (fov / 2.0f) * (2PI / 360.0f)
    // x_max = y_max * (window_width / window_height)
    float32 y_max = n * tanf(fov * PI / 360.0f);
    float32 x_max = y_max * aspect_ratio;
    return get_frustum(-x_max, x_max, y_max, -y_max, n, f);
}

inline Matrix_4x4
orthographic_projection(float32 l, float32 r, float32 t, float32 b, float32 n, float32 f) {
    if (l == r || t == b || n == f) {
        logprint("orthographic_projection()", "Invalid arguments");
        return {};
    }
    return {
        2.0f / (r - l),  0,              0,              0,
        0,               2.0f / (t - b), 0,              0,
        0,               0,             -2.0f / (f - n), 0,
        -((r+l)/(r-l)), -((t+b)/(t-b)), -((f+n)/(f-n)),  1
    };
}

#elif VULKAN

inline Matrix_4x4
perspective_projection(float32 fov, float32 aspect_ratio, float32 n, float32 f) {
    float32 y_max = n * tanf(fov * PI / 360.0f);
    float32 x_max = y_max * aspect_ratio;
    return get_frustum(-x_max, x_max, -y_max, y_max, n, f);
}

inline Matrix_4x4
orthographic_projection(float32 l, float32 r, float32 b, float32 t, float32 n, float32 f) {
    if (l == r || t == b || n == f) {
        logprint("orthographic_projection()", "Invalid arguments");
        return {};
    }
    return {
        2.0f / (r - l),  0,              0,              0,
        0,               2.0f / (t - b), 0,              0,
        0,               0,             -2.0f / (f - n), 0,
        -((r+l)/(r-l)), -((t+b)/(t-b)), -((f+n)/(f-n)),  1
    };
}

#endif // OPENGL / VULKAN



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
look_at(const Vector3 &position, const Vector3 &target, const Vector3 &up) {
    Vector3 f = normalized(target - position) * -1.0f;
    Vector3 r = cross_product(up, f);
    if (r == 0) return identity_m4x4();
    normalize(r);
    Vector3 u = normalized(cross_product(f, r));
    Vector3 t = {-dot_product(r, position), -dot_product(u, position), -dot_product(f, position)};
    
    return {
        r.x, u.x, f.x, 0,
        r.y, u.y, f.y, 0,
        r.z, u.z, f.z, 0,
        t.x, t.y, t.z, 1
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
inverse(Matrix_4x4 m) {
    
}

#endif // TYPES_MATH_H