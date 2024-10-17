struct Rect {
  union {
    Vector2 p1;
    Vector2 coords;
  };
  union {
    Vector2 p2;
    Vector2 dim;
  };
};

struct GFX {
  Vector2_s32 window_dim;
  float32 aspect_ratio;
  Rect viewport;
  Rect scissor;

  bool8 vsync = true;

  // API3D
  D3D12_State d3d12;
  //Vulkan_State vulkan;
};

GFX gfx = {};

typedef enum {
  VECTOR2_TYPE,
  VECTOR3_TYPE,
  VECTOR4_TYPE
} Vector_Format;

struct Vertex_Attribute {
  Vector_Format format;
  u32 offset;
};

struct Vertex_Info {
  Vertex_Attribute *attributes;
  u32 attributes_count;
  u32 size;
};

internal u32
vertex_info_get_size(Vertex_Info *info) {
	u32 vertex_size = 0;
	for (u32 i = 0; i < info->attributes_count; i++) {
		switch(info->attributes[i].format) {
			case VECTOR2_TYPE: vertex_size += sizeof(Vector2); break;
			case VECTOR3_TYPE: vertex_size += sizeof(Vector3); break;
			case VECTOR4_TYPE: vertex_size += sizeof(Vector4); break;
		}
	}
	return vertex_size;
}

struct Vertex_PC {
  Vector3 position;
  Vector4 color;
};

Vertex_Attribute Vector_PC_Attributes[] = {
  { VECTOR3_TYPE, offsetof(Vertex_PC, position) },
  { VECTOR4_TYPE, offsetof(Vertex_PC, color) }
};
  
struct Mesh {
  Vertex_Info vertex_info;
    
  void *vertices;
  u32 vertices_count;

  u32 *indices;
  u32 indices_count;

  D3D12_Mesh gpu;
};

