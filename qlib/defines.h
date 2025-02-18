#ifndef DEFINES_H
#define DEFINES_H

/*
Compiler Defines

OS:
OS_WINDOWS
OS_LINUX

API:
SDL
STEAM

GFX:
OPENGL
VULKAN

COMPILE_TYPE:
DEBUG
*/

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

#define DEG2RAD  0.0174533f
#define DEG2RADD 0.0174533
#define PI       3.141592653593f
#define EPSILON  0.00001f

#define MAX_FRAMES_IN_FLIGHT  2

#define ASSERT(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define ARRAY_COUNT(n)     (sizeof(n) / sizeof(n[0]))
#define ARRAY_MALLOC(t, n)       ((t*)platform_malloc      (n * sizeof(t))) // WARNING!!!! Make sure n is in brackets if operation
#define ARRAY_MALLOC_CLEAR(t, n) ((t*)platform_malloc_clear(n * sizeof(t))) // WARNING!!!! Make sure n is in brackets if operation

#define X_AXIS { 1, 0, 0 }
#define Y_AXIS { 0, 1, 0 }
#define Z_AXIS { 0, 0, 1 }

#ifdef OS_WINDOWS
#define WIN32_FUNC(n) os_##n
#define OS_EXT(n) win32_##n
#elif OS_LINUX
#define LINUX_FUNC(n) os_##n
#define OS_EXT(n) linux_##n
#endif // OS

#if OPENGL
#define API3D_EXT(n) opengl_##n
#elif VULKAN
#define API3D_EXT(n) vulkan_##n
#elif DX12
#define API3D_EXT(n) dx12_##n
#endif // OPENGL / VULKAN / DX12

#define OS_FUNC(r, n, ...) r OS_EXT(n)(__VA_ARGS__); r (*os_##n)(__VA_ARGS__) = &OS_EXT(n)

#endif // DEFINES_H
