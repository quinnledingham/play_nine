struct File {
  void *memory;
  u32 size;
};

struct Bitmap {

};

enum shader_stages {
  SHADER_STAGE_VERTEX,
  SHADER_STAGE_TESSELLATION_CONTROL,
  SHADER_STAGE_TESSELLATION_EVALUATION,
  SHADER_STAGE_GEOMETRY,
  SHADER_STAGE_FRAGMENT,
  SHADER_STAGE_COMPUTE,

  SHADER_STAGES_COUNT
};

const char *shader_file_types[] = {
  ".vert",
  ".tcs",
  ".tes",
  ".gs",
  ".frag",
  ".compute",
};

// lines up with enum shader_stages
const u32 shaderc_glsl_file_types[] = { 
  shaderc_glsl_vertex_shader,
  shaderc_glsl_tess_control_shader ,
  shaderc_glsl_tess_evaluation_shader,
  shaderc_glsl_geometry_shader,
  shaderc_glsl_fragment_shader,
  shaderc_glsl_compute_shader,
};

struct Shader_File {
  const char *filename;
  u32 stage;
  File glsl;
  File spirv;
};

struct Shader {
  Shader_File files[SHADER_STAGES_COUNT];
};

//
// Asset Manager
//

enum ASSET_TYPE {
  ASSET_TYPE_SHADER,
  ASSET_TYPE_BITMAP,

  ASSET_TYPE_COUNT
};

u32 asset_type_sizes[] = {
  sizeof(Shader),
  sizeof(Bitmap),
};

const char *asset_folders[] = {
  "../assets/shaders/",
  "../assets/bitmaps/",
};

const char* get_asset_folder(u32 type) {
  return asset_folders[type];
}

struct Asset_Array {
  void *memory;
  u32 type; // ASSET_TYPE
  u32 size; // how many bytes are allocated
  u32 count; // how many of that asset type allocated
};

struct Asset_Load_Info {
  u32 type;
  u32 id;
  std::string filename;
};

struct Assets {
  Shader *shaders;
  u32 shader_index;
  u32 shaders_count;

  std::vector<Asset_Load_Info> load_info; // same length as *files
  File *files;

  Asset_Array arrays[ASSET_TYPE_COUNT];
};

/*
How to add a asset:

1. 
Add the asset id you want to the appropriate enum (Bitmap_IDs, Sound_IDs).

2. 
Add it to list of assets to load in init(), this relates the id to the filename, and confirms
what type of asset it is.

3.
Should be able to access it now using the id and proper find_<type> function

*/

/*
Asset Manager Philosophy

Assets holds arrays of each of the assets that can be accessed from using
enums that are specified. This may mean there is some unused space, not really
though as long as you don't specify a bunch of unused enums.

Assets are centralized so that they are easy to export to a asset file. It should
be easy to save all of the files and assets since they are all managed in similar
ways in the assets struct.

These assets arrays also makes it easy to say reload all of the shaders when
the swap chain has been recreated or any other time you want to iterate over
a kind of asset.
*/