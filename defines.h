#ifndef DEFINES_H
#define DEFINES_H

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s8  bool8;
typedef s32 bool32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  float32;
typedef double float64;

#define FALSE 0
#define TRUE 1

#define internal      static
#define local_persist static
#define global        static

#define DEG2RAD 0.0174533f
#define PI      3.14159265359f
#define EPSILON 0.00001f

#define ASSERT(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define ARRAY_COUNT(n)     (sizeof(n) / sizeof(n[0]))
#define ARRAY_MALLOC(t, n) ((t*)platform_malloc(n * sizeof(t)))

#endif // DEFINES_H