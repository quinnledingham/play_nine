#ifndef GOLFO_ASSETS_H
#define GOLFO_ASSETS_H

//
// Fonts
//

enum Font_Ids {
  FONT_LIMELIGHT,
  FONT_ROBOTO_MONO,

  FONT_COUNT
};

Asset_Load font_loads[] = {
  { FONT_LIMELIGHT, {"Limelight-Regular.ttf"} },
  { FONT_ROBOTO_MONO, {"RobotoMono.ttf"} },
};

Assets_Load_Info fonts_info = {
  &assets.fonts, 
  font_loads, 
  ARRAY_COUNT(font_loads), 
  AT_FONT
};

//
// Textures
//

enum Texture_Ids {
  TEXTURE_LANA,

  TEXTURE_BACK,
  TEXTURE_FRONT,
  TEXTURE_FRONT_0,
  TEXTURE_FRONT_11,
  TEXTURE_FRONT_12,

  TEXTURE_COUNT
};

Asset_Load texture_loads[] = {
  { TEXTURE_LANA, {"lana.jpeg"} },

  { TEXTURE_BACK, {"back.png"} },
  { TEXTURE_FRONT, {"front.png"} },
  { TEXTURE_FRONT_0, {"front0.png"} },
  { TEXTURE_FRONT_11, {"front11.png"} },
  { TEXTURE_FRONT_12, {"front12.png"} },
};

Assets_Load_Info textures_info = {
  &assets.textures, 
  texture_loads, 
  ARRAY_COUNT(texture_loads), 
  AT_TEXTURE
};

/*
  Pipelines
*/

enum Pipeline_Ids {
  PIPELINE_2D,
  PIPELINE_3D,
  PIPELINE_NOISE,
  PIPELINE_NOISE_TEXTURE,

  PIPELINE_COUNT
};

Asset_Load pipeline_loads[] = {
  { PIPELINE_2D, {"2D.vs", "2D.fs"} },
  { PIPELINE_3D, {"3D.vs", "3D.fs"} },
  { PIPELINE_NOISE, {"2D.vs", "noise.fs"} },
  { PIPELINE_NOISE_TEXTURE, { "noise.comp" } },
};

Assets_Load_Info pipeline_info = {
  &assets.pipelines, 
  pipeline_loads, 
  ARRAY_COUNT(pipeline_loads), 
  AT_SHADER
};

//
// Layouts
//

enum GFX_Layout_IDs {
  // vertex
  GFXID_SCENE,

  // fragment
  GFXID_GLOBAL,
  GFXID_LOCAL,
  GFXID_MATERIAL,
  GFXID_TEXTURE,

  // compute
  GFXID_NOISE_TEXTURE,

  GFXID_COUNT
};

internal void
gfx_define_layouts() {
  gfx.layouts_count = GFXID_COUNT;
  gfx.layouts = ARRAY_MALLOC(GFX_Layout, gfx.layouts_count);
  SDL_memset(gfx.layouts, 0, sizeof(GFX_Layout) * gfx.layouts_count);

  gfx.layouts[GFXID_SCENE].set_number = 0;

  gfx.layouts[GFXID_GLOBAL].set_number = 1;
  gfx.layouts[GFXID_LOCAL].set_number = 2;
  gfx.layouts[GFXID_MATERIAL].set_number = 4;
  gfx.layouts[GFXID_TEXTURE].set_number = 3;
  gfx.layouts[GFXID_NOISE_TEXTURE].set_number = 0;

  gfx.layouts[GFXID_SCENE].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, 1, sizeof(Scene)); // 2D.vert

  gfx.layouts[GFXID_GLOBAL].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Global_Shader)); // 2D.vert
  gfx.layouts[GFXID_LOCAL].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Local)); // color.frag, text.frag
  gfx.layouts[GFXID_MATERIAL].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Material_Shader)); // 3D.frag
  gfx.layouts[GFXID_TEXTURE].add_binding(0, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, 1, 0); // texture.frag
  
  gfx.layouts[GFXID_NOISE_TEXTURE].add_binding(0, DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_STAGE_COMPUTE, 1, 0); // noise.comp

  for (u32 i = 0; i < gfx.layouts_count; i++) {
    vulkan_setup_layout(&gfx.layouts[i]);
  }
}

internal void 
gfx_add_layouts_to_shaders() {
  {
    Pipeline *shader = find_pipeline(PIPELINE_2D);
    shader->vertex_info = Vertex_XU::info();

    shader->set.add_layout(&gfx.layouts[GFXID_SCENE]);
    shader->set.add_push(SHADER_STAGE_VERTEX, sizeof(Object));

    shader->set.add_layout(&gfx.layouts[GFXID_GLOBAL]);
    shader->set.add_layout(&gfx.layouts[GFXID_LOCAL]);
    shader->set.add_layout(&gfx.layouts[GFXID_TEXTURE]);
  }

  {
    Pipeline *shader = find_pipeline(PIPELINE_3D);
    shader->vertex_info = Vertex_XNU::info();

    shader->set.add_layout(&gfx.layouts[GFXID_SCENE]);
    shader->set.add_push(SHADER_STAGE_VERTEX, sizeof(Object));

    shader->set.add_layout(&gfx.layouts[GFXID_GLOBAL]);
    shader->set.add_layout(&gfx.layouts[GFXID_LOCAL]);
    shader->set.add_layout(&gfx.layouts[GFXID_TEXTURE]);
    shader->set.add_layout(&gfx.layouts[GFXID_MATERIAL]);
  }

  {
    Pipeline *shader = find_pipeline(PIPELINE_NOISE);
    shader->vertex_info = Vertex_XU::info();

    shader->set.add_layout(&gfx.layouts[GFXID_SCENE]);
    shader->set.add_push(SHADER_STAGE_VERTEX, sizeof(Object));

    shader->set.add_layout(&gfx.layouts[GFXID_GLOBAL]);
    shader->set.add_layout(&gfx.layouts[GFXID_LOCAL]);
    shader->set.add_layout(&gfx.layouts[GFXID_TEXTURE]);
  }

  {
    Pipeline *shader = find_pipeline(PIPELINE_NOISE_TEXTURE);
    shader->set.add_layout(&gfx.layouts[GFXID_NOISE_TEXTURE]);
    shader->set.add_layout(&gfx.layouts[GFXID_GLOBAL]);
    shader->set.add_layout(&gfx.layouts[GFXID_LOCAL]);
  }
}


#endif // GOLFO_ASSETS_H