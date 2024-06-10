#ifndef CHAR_ARRAY_H
#define CHAR_ARRAY_H

//
// string
//

inline bool8
equal(const char* a, const char *b)
{
    if (a == 0 && b == 0) return true;
    if (a == 0 || b == 0) return false;
    
    int i = 0;
    do
    {
        if (a[i] != b[i])
            return false;
    } while(a[i] != 0 && b[i++] != 0);
    
    return true;
}

inline bool8
equal_start(const char *longer, const char *shorter)
{
    if (longer == 0 && shorter == 0) return true;
    if (longer == 0 || shorter == 0) return false;

    u32 i = 0;
    do
    {
        if (shorter[i] == 0) break;
        if (longer[i] != shorter[i]) 
            return false;
    } while(longer[i] != 0 && shorter[i++] != 0);
    
    return true;
}

inline u32
get_length(const char *string)
{
    if (string == 0)
        return 0;
    
    u32 length = 0;
    const char *ptr = string;
    while(*ptr != 0)
    {
        length++;
        ptr++;
    }
    return length;
}


// assumes that dest is big enough for src
inline void
copy_char_array(char *dest, const char *src)
{
    const char *ptr = src;
    while(*ptr != 0)
    {
        *dest++ = *ptr;
        ptr++;
    }
}

internal const char*
char_array_insert(const char *og_string, u32 position, const char *new_string)
{
    u32 og_length = get_length(og_string);
    u32 new_length = get_length(new_string);
    u32 total_length = og_length + new_length;
    const char *result = (const char *)platform_malloc(total_length + 1);
    char *ptr = (char *)result;

    u32 result_index = 0;
    u32 og_string_index = 0;
    for (og_string_index;           og_string_index <   position;  og_string_index++) ptr[result_index++] = og_string[og_string_index];
    for (u32 new_string_index = 0; new_string_index < new_length; new_string_index++) ptr[result_index++] = new_string[new_string_index];
    for (og_string_index;           og_string_index < og_length;   og_string_index++) ptr[result_index++] = og_string[og_string_index];
    ptr[result_index] = 0;

    return result;
}

internal const char*
char_array_concat(const char *left, const char *right) {
    u32 left_length = get_length(left);
    u32 right_length = get_length(right);
    u32 total_length = left_length + right_length;
    char *result = (char *)platform_malloc(total_length + 1);

    u32 result_index = 0;
    for (u32 index = 0; index < left_length; index++)
        result[result_index++] = left[index];
    for (u32 index = 0; index < right_length; index++)
        result[result_index++] = right[index];

    result[result_index] = 0;
    return result;
}

inline char*
string_malloc(const char *string)
{
    if (string == 0) return 0;
    u32 length = get_length(string);
    char* result = (char*)platform_malloc(length + 1);
    for (u32 i = 0; i < length; i++) result[i] = string[i];
    result[length] = 0;
    return result;
}

inline const char*
string_malloc_length(const char *string, u32 length)
{
    if (string == 0) return 0;
    char* result = (char*)platform_malloc(length + 1);
    for (u32 i = 0; i < length; i++) result[i] = string[i];
    result[length] = 0;
    return result;
}

// converts n number of chars to a string
// ex) chtos(3, a, b, c) returns "abc"
inline char*
chtos(int n, ...)
{
    char* s = (char*)platform_malloc(n + 1);
    platform_memory_set(s, 0, n + 1);
    
    va_list ptr;
    va_start(ptr, n);
    for (int i = 0; i < n; i++)
    {
        s[i] = va_arg(ptr, int);
    }
    
    return s;
}

inline u32
get_digits(s32 in) {
    u32 digits = 0;
    do {
        digits++;
        in = in / 10;
    } while(in != 0);
    return digits;
}

inline u32
s32_to_char_array(char *buffer, u32 size, s32 in) {
    u32 ret = snprintf(buffer, size, "%d", in);
    if (ret < 0) {
        logprint("s32_to_char_array()", "snprintf() failed\n");
        return 0;
    }
    if (ret >= size)
        logprint("s32_to_char_array()", "snprintf(): result was truncated\n");
    return ret;
}

inline const char*
s32_to_char_array(s32 in) {
    u32 size = 10;
    char *buffer = (char*)platform_malloc(size);
    platform_memory_set(buffer, 0, size);
    u32 ret = snprintf(buffer, size, "%d", in);
    if (ret < 0) {
        logprint("s32_to_char_array()", "snprintf() failed\n");
        return 0;
    }
    if (ret >= size) logprint("s32_to_char_array()", "snprintf(): result was truncated\n");
    return buffer;
}

inline const char*
u32_to_char_array(u32 in) {
    return s32_to_char_array((s32)in);
}

// ptr must point to first char of int
inline const char*
char_array_to_s32(const char *ptr, s32 *result) {
    u32 sign = 1;
    s32 num = 0;

    if (*ptr == '-') {
        sign = -1;
        ptr++;
    }

    while (isdigit(*ptr)) num = 10 * num + (*ptr++ - '0');
    *result = sign * num;

    return ptr;
}

inline const char*
char_array_to_u32(const char *ptr, u32 *result)
{   
    s32 num = 0;
    ptr = char_array_to_s32(ptr, &num);
    *result = (u32)num;
    return ptr;
}

inline const char *
float_to_char_array(float32 f) {
    u32 size = 64;
    char *buffer = (char*)platform_malloc(size);
    platform_memory_set(buffer, 0, size);
    u32 ret = snprintf(buffer, size, "%f", f);
    if (ret < 0) {
        logprint("float_to_char_array()", "ftos() failed\n");
        return 0;
    }
    if (ret >= size) logprint("float_to_char_array(float32 f)", "ftos(): result was truncated\n");
    return buffer;
}

inline void
float_to_char_array(float32 f, char *buffer, u32 buffer_size) {
    u32 ret = snprintf(buffer, buffer_size, "%f", f);
    if (ret < 0) {
        logprint("float_to_char_array(float32 f, char *buffer, u32 buffer_size)", "ftos() failed\n");
        return;
    }
    if (ret >= buffer_size) logprint("float_to_char_array(float32 f, char *buffer, u32 buffer_size)", "ftos(): result was truncated\n");
}

// char_array_to_float

#define MAX_POWER 20

global const
double POWER_10_POS[MAX_POWER] =
{
    1.0e0,  1.0e1,  1.0e2,  1.0e3,  1.0e4,  1.0e5,  1.0e6,  1.0e7,  1.0e8,  1.0e9,
    1.0e10, 1.0e11, 1.0e12, 1.0e13, 1.0e14, 1.0e15, 1.0e16, 1.0e17, 1.0e18, 1.0e19,
};

global const
double POWER_10_NEG[MAX_POWER] =
{
    1.0e0,   1.0e-1,  1.0e-2,  1.0e-3,  1.0e-4,  1.0e-5,  1.0e-6,  1.0e-7,  1.0e-8,  1.0e-9,
    1.0e-10, 1.0e-11, 1.0e-12, 1.0e-13, 1.0e-14, 1.0e-15, 1.0e-16, 1.0e-17, 1.0e-18, 1.0e-19,
};

inline bool8
is_exponent(char c)
{
    return (c == 'e' || c == 'E');
}

// returns the point where it stopped reading chars
// reads up until it no longer looks like a number
// writes the result it got to where result pointer
inline const char*
char_array_to_float32(const char *ptr, float32 *result)
{
    float64 sign = 1.0;
    float64 num  = 0.0;
    float64 fra  = 0.0;
    float64 div  = 1.0;
    u32 eval = 0;
    const float64* powers = POWER_10_POS;

    switch (*ptr)
    {
        case '+': sign =  1.0; ptr++; break;
        case '-': sign = -1.0; ptr++; break;
    }

    while (isdigit(*ptr)) num = 10.0 * num + (double)(*ptr++ - '0');

    if (*ptr == '.') ptr++;

    while (isdigit(*ptr))
    {
        fra  = 10.0 * fra + (double)(*ptr++ - '0');
        div *= 10.0;
    }

    num += fra / div;

    if (is_exponent(*ptr))
    {
        ptr++;

        switch (*ptr)
        {
            case '+': powers = POWER_10_POS; ptr++; break;
            case '-': powers = POWER_10_NEG; ptr++; break;
        }

        while (isdigit(*ptr)) eval = 10 * eval + (*ptr++ - '0');

        num *= (eval >= MAX_POWER) ? 0.0 : powers[eval];
    }

    *result = (float32)(sign * num);

    return ptr;
}

internal const char*
get_path(const char *file)
{
    u32 length = get_length(file);
    char *ptr = (char*)file;
    ptr += length;

    u32 path_length = length;
    ptr--;
    while(*ptr != '/')
    {
        ptr--;
        path_length--;
    }

    char *path = (char*)platform_malloc(path_length + 1);
    for (u32 i = 0; i < path_length; i++)
    {
        path[i] = file[i];
    }
    path[path_length] = 0;

    return (const char*)path;
}

internal const char*
get_filename(const char *filepath) {
    if (filepath == 0)
        return 0;

    u32 length = get_length(filepath);
    char *ptr = (char*)filepath;
    ptr += length;

    u32 name_length = 0;
    while(*ptr != '/') {
        ptr--;
        name_length++;
    }
    ptr++;
    name_length--;

    char *filename = (char*)platform_malloc(name_length + 1);
    for (u32 i = 0; i < name_length; i++) {
        filename[i] = ptr[i];
    }
    filename[name_length] = 0;

    return filename;
}

struct Pair {
    u32 key;
    const char *value;
};

inline bool8
pair_is_value(const Pair *pairs, u32 num_of_pairs, const char *value) {
    for (u32 i = 0; i < num_of_pairs; i++) {
        if (equal(pairs[i].value, value))
            return true;
    }
    return false;
}

inline const char*
pair_get_value(const Pair *pairs, u32 num_of_pairs, u32 key) {
    for (u32 i = 0; i < num_of_pairs; i++) {
        if (pairs[i].key == key) return pairs[i].value;
    }
    return 0;
}

inline u32
pair_get_key(const Pair *pairs, u32 num_of_pairs, const char *value) {
    for (u32 i = 0; i < num_of_pairs; i++) {
        if (equal(pairs[i].value, value)) return pairs[i].key;
    }
    return num_of_pairs; // returns out of range int
}

#endif // CHAR_ARRAY_H
