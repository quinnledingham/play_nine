#include <stdarg.h>
#include <cstdint>
#include <ctype.h>
#include <stdio.h>

#define internal      static
#define local_persist static
#define global        static

#include "types.h"

void *platform_malloc(u32 size);
void platform_free(void *ptr);
void platform_memory_set(void *dest, s32 value, u32 num_of_bytes);

#include "print.h"
#include "char_array.h"
#include "assets.h"

#include "application.h"
#include "play_nine.h"

bool8 update(App *app) {
	print("%f\n", app->time.frames_per_s);

	return 0;
}