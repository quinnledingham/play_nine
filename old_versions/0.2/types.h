#ifndef TYPES_H
#define TYPES_H

enum Type_Enum {
  TYPE_U32,
  TYPE_S32,
  TYPE_FLOAT32,
  TYPE_FLOAT64,

  TYPE_COUNT
};

union Vector2 {
	struct {
		float32 x, y;
	};
	struct {
		float32 u, v;
	};
	struct {
		float32 width, height;
	};
	struct {
		float32 w, h;
	};
	float32 E[2];
};

union Vector2_s32 {
	struct {
		s32 x, y;
	};
	struct {
		s32 u, v;
	};
	struct {
		s32 width, height;
	};
	struct {
		s32 w, h;
	};
	s32 E[2];
};

union Vector3 {
	struct {
		float32 x, y, z;
	};
	struct {
		float32 r, g, b;
	};
	struct {
		float32 w, p, k;
	};
	struct {
		float32 omega, phi, kappa;
	};
	struct {
		float32 roll, pitch, yaw;
	};
	float32 E[3];

	Vector2 xz() {
		return { x, z };
	}
};

union Vector4 {
	struct {
		float32 x, y, z, w;
	};
	struct {
		float32 r, g, b, a;
	};
	struct {
		Vector3 rgb;
	};
	struct {
		Vector2 xy;
	};
	struct {
		Vector3 xyz;
	};
	float32 E[4];
};

struct Buffer {
	void *memory;
	u32 size;

	char* str() { return (char *)memory; }
	void clear() { memset(memory, 0, size); }
  	bool8 in(void *ptr) {
	  s64 diff = (char*)ptr - (char*)memory;
	  return 0 <= diff && diff < size;
	}
};

Buffer blank_buffer(u32 size);
Buffer buffer_copy_create(u32 size, Buffer copy);
void destroy_buffer(Buffer *buffer);

internal void
buffer_resize(Buffer *b, u32 new_size) {
	Buffer new_buffer = buffer_copy_create(new_size, *b);
	destroy_buffer(b);	
	*b = new_buffer;
}


union Quaternion {
    struct {
        float32 x, y, z, w;
    };
    struct {
        Vector3 vector;
        float32 scalar;
    };
    float32 E[4];
};

struct Matrix_3x3 {
	float32 E[3][3];
};

struct Matrix_4x4 {
	union {
		float32 E[4][4];
		float32 F[16];
	};
};

union Color_RGB {
	struct {
		u8 r, g, b;
	};
	u8 E[3];
};

union Color_RGBA {
	struct {
		u8 r, g, b, a;
	};
	u8 E[4];
};
/*
union Pose {
    struct {
        union {
            struct {
                float32 x, y, z;
            };
            Vector3 position;
        };
        union {
            struct {
                float32 omega, phi, kappa;
            };
            struct {
                float32 roll, pitch, yaw;
            };
            struct {
                float32 w, p, k;
            };
            Vector3 orientation;
        };
		Vector3 padding; // to match Transform
    };
    float32 E[9];
};
*/

struct Transform {
	union {
		struct {
	        union {
	            struct {
	                float32 x, y, z;
	            };
	            Vector3 position;
	        };
	        union {
	            struct {
	                float32 omega, phi, kappa;
	            };
	            struct {
	                float32 roll, pitch, yaw;
	            };
	            struct {
	                float32 w, p, k;
	            };
	            Vector3 orientation;
	        };
		};
		float32 P[6];
	};
	Vector3 scale = { 1.0f, 1.0f, 1.0f };
};


typedef Transform Pose;

#endif // TYPES_H