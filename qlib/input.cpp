Bitmap keyboard_prompt_texture;

internal void
create_input_prompt_texture(Input_Prompt *prompts, u32 num_of_prompts, const char *folder_path, const char *key_type, const char *filename) {
  
  atlas = create_texture_atlas(500, 500, 4, GFX_ID_TEXTURE);
  
  u32 folder_path_length = get_length(folder_path);
  char filepath[100];
  platform_memory_set(filepath, 0, 100);
  platform_memory_copy(filepath, folder_path, folder_path_length);

  for (u32 prompt_index = 0; prompt_index < num_of_prompts; prompt_index++) {
    char *path = filepath + folder_path_length;
    platform_memory_set(path, 0, 100 - folder_path_length);
    
    const char *filename = prompts[prompt_index].filename;
    u32 filename_length = get_length(filename);
    platform_memory_copy(path, filename, filename_length);
    path += filename_length;
    platform_memory_copy(path, key_type, get_length(key_type));

    File file = load_file(filepath);
    Bitmap prompt_bitmap = load_bitmap(file, false);

    texture_atlas_add(&atlas, &prompt_bitmap);
  }

  write_bitmap(&atlas.bitmap, filename);
  texture_atlas_init_gpu(&atlas);
}

internal u32
find_input_prompt_index(s32 id, Input_Prompt *prompts, u32 num_of_prompts) {
  for (u32 prompt_index = 0; prompt_index < num_of_prompts; prompt_index++) {
    if (prompts[prompt_index].id == id)
      return prompt_index;
  }

  logprint("find_input_prompt_index", "did not find prompt index");
  return 0;
}

internal void
draw_input_prompt(Vector3 coords, Button button) {
  GFX_Object object = {};
  Quaternion rotation = get_rotation(0.0f, { 0, 0, 1 });
  Vector3 dim = { 100, 100, 1 };

  gfx_bind_shader("PROMPT");
  
  Descriptor desc = render_get_descriptor_set(3);
  render_set_bitmap(&desc, &keyboard_prompt_texture);
  render_bind_descriptor_set(desc);
  
  object.index = find_input_prompt_index(button.ids[0].id, keyboard_prompts, ARRAY_COUNT(keyboard_prompts));
  
  coords.x += dim.x / 2.0f;
  coords.y += dim.y / 2.0f; // coords = top left corner
  object.model = create_transform_m4x4(coords, rotation, dim);
  render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(GFX_Object));
  
  render_draw_mesh(&shapes.rect_mesh);
}

