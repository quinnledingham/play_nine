internal void
create_input_prompt_atlas(Texture_Atlas *atlas, Input_Prompt *prompts, u32 num_of_prompts, const char *folder_path, const char *key_type, const char *filename) {
  
  *atlas = create_texture_atlas(1500, 1000, 4);
  
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

    texture_atlas_add(atlas, &prompt_bitmap);
  }  
}

internal u32
find_input_prompt_index(s32 id, Input_Prompt *prompts, u32 num_of_prompts) {
  for (u32 prompt_index = 0; prompt_index < num_of_prompts; prompt_index++) {
    if (prompts[prompt_index].id == id)
      return prompt_index;
  }

  logprint("find_input_prompt_index", "did not find prompt index\n");
  return 0;
}

internal void
draw_input_prompt(Button button, Vector2 coords, Vector2 dim) {
  u32 atlas_index = 0;
  if (app_input->active == CONTROLLER_INPUT) {
    atlas_index = 1;
  }

  u32 id_index = 0;
  for (id_index; id_index < 3; id_index++) {
    if (app_input->active == button.ids[id_index].type) {
      break;
    }
  }

  u32 index = 0;
  if (app_input->active == KEYBOARD_INPUT) {  
    index = find_input_prompt_index(button.ids[id_index].id, keyboard_prompts, ARRAY_COUNT(keyboard_prompts));
  } else if (app_input->active == CONTROLLER_INPUT) {
    index = find_input_prompt_index(button.ids[id_index].id, xbox_prompts, ARRAY_COUNT(xbox_prompts));
  }
  texture_atlas_draw(input_prompt_atlases[atlas_index], index, coords, dim);
}

internal Vector2
get_input_prompt_dim(const char *string, Font *font, Vector2 dim) {
  float32 pixel_height = float32(dim.y);
  Vector2 padding = {5.0f, 0.0f};
  String_Draw_Info string_info = gfx.get_string_draw_info(font, string, -1, pixel_height);
  return dim + string_info.dim + padding;
}

internal void
draw_input_prompt(const char *string, Font *font, Button button, Vector2 coords, Vector2 dim) {
  float32 pixel_height = float32(dim.y);
  Vector4 color = { 255, 255, 255, 1 };

  draw_input_prompt(button, coords, dim);

  String_Draw_Info string_info = gfx.get_string_draw_info(font, string, -1, pixel_height);
  coords.x += dim.x + 5.0f;
  coords.y += (dim.y / 2.0f) - (string_info.dim.y / 2.0f);
  coords.y += string_info.baseline.y;
  gfx.draw_string(font, string, coords, pixel_height, color);
}
