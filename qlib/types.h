#ifndef TYPES_H
#define TYPES_H

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
	s32 E[2];
};

union Vector3 {
	struct {
		float32 x, y, z;
	};
	struct {
		float32 r, g, b;
	};
	float32 E[3];
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
		Vector3 xyz;
	};
	float32 E[4];
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

/*
0 -> 0 1 2 3
1 -> 4 5 6 7
2 -> 8 9 1 2
3 -> 3 4 5 6
*/

struct Matrix_3x3 {
	float32 E[3][3];
};

struct Matrix_4x4 {
	union {
		float32 E[4][4];
		float32 F[16];
	};
};

#endif // TYPES_H
