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

#define ARRAY_COUNT(n) (sizeof(n) / sizeof(n[0]))
#define ARRAY_MALLOC(t, n) ((t*)malloc(n * sizeof(t))) // WARNING!!!! Make sure n is in brackets if operation

#define internal      static
#define local_persist static
#define global        static

#define FALSE 0
#define TRUE 1

#define SUCCESS 0
#define FAILURE 1

#define ASSERT(Expression) if(!(Expression)) {*(int *)0 = 0;}

#define PI       3.141592653593f
#define EPSILON  0.00001f
#define DEG2RAD  0.0174533f
#define RAD2DEG  57.295780f

#define X_AXIS { 1, 0, 0 }
#define Y_AXIS { 0, 1, 0 }
#define Z_AXIS { 0, 0, 1 }

#ifdef SDL
#define SDL_GFX_FUNC(n) gfx_##n
#endif // SDL

#endif // DEFINES_H
