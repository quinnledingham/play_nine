Bitmap keyboard_prompt_texture;

internal void
create_input_prompt_texture(Input_Prompt *prompts, u32 num_of_prompts, const char *folder_path, const char *key_type, const char *filename) {
  u32 columns = 10;
  u32 rows = (u32)ceilf(float32(num_of_prompts) / float32(columns));
  
  Bitmap bitmap = {};
  bitmap.width = 100 * columns; // each prompt bitmap is 100; so 10 per row
  bitmap.height = 100 * rows;
  bitmap.channels = 4;
  bitmap.pitch = bitmap.width * bitmap.channels;
  u32 bitmap_size = bitmap.width * bitmap.height * bitmap.channels;
  bitmap.memory = (u8 *)platform_malloc(bitmap_size);
  platform_memory_set(bitmap.memory, 0, bitmap_size);

  //const char *key_type = "_Key_Dark.png";
  //char *folder_path = "../xelu/Keyboard & Mouse/Dark/";
  
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

    Vector2_s32 position = { ((s32)prompt_index % 10) * 100, ((s32)prompt_index / 10) * 100 };
    copy_blend_bitmap(bitmap, prompt_bitmap, position, { 255, 255, 255 });
  }

  write_bitmap(&bitmap, filename);

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
  render_bind_pipeline(&pipelines[PIPELINE_PROMPT]);

  Descriptor desc = render_get_descriptor_set(&layouts[3]);
  render_set_bitmap(&desc, &keyboard_prompt_texture);
  render_bind_descriptor_set(desc);
  
  object.index = find_input_prompt_index(button.ids[0].id, keyboard_prompts, ARRAY_COUNT(keyboard_prompts));
  
  coords.x += dim.x / 2.0f;
  coords.y += dim.y / 2.0f; // coords = top left corner
  object.model = create_transform_m4x4(coords, rotation, dim);
  render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(GFX_Object));
  
  render_draw_mesh(&shapes.rect_mesh);
}
