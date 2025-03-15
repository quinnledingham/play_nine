void GFX::destroy_frame_resources() {
  device_wait_idle();

  cleanup_swap_chain();

  destroy_texture(&color_texture);
  destroy_texture(&depth_texture);

  destroy_render_pass(draw_render_pass);
  destroy_render_pass(present_render_pass);

  swap_chain_created = FALSE;
}

void GFX::create_frame_resources() {
  //destroy_frame_resources();
  if (swap_chain_created) {
    log_error("Trying to recreate swap chain when it hasnt been destroyed yet\n");
    return;
  }

  if (anti_aliasing)
    msaa_samples = get_max_usable_sample_count();
  else
    msaa_samples = VK_SAMPLE_COUNT_1_BIT;

  create_swap_chain(window.dim, vsync);
  create_render_image_views(window.resolution);
  create_draw_render_pass(resolution_scaling, anti_aliasing);
  create_present_render_pass();
  create_depth_resources(window.resolution);
  create_color_resources(window.resolution);

  // Get the size of the offscreen draw buffer
  if (resolution_scaling) {
    VkExtent2D extent = {};
    extent.width = window.resolution.width;
    extent.height = window.resolution.height;
    draw_extent = extent;
  } else {
    draw_extent = swap_chain_extent;
  }

  create_draw_framebuffers(resolution_scaling, anti_aliasing);
  create_swap_chain_framebuffers();
  
  clear_values[1].depthStencil = {1.0f, 0};

  //vulkan_recreate_pipelines(assets);  
  swap_chain_created = TRUE;
}

void GFX::init() {
  u32 layouts_count = 2;
  layouts = ARRAY_MALLOC(GFX_Layout, layouts_count);
  memset(layouts, 0, sizeof(GFX_Layout) * layouts_count);

  layouts[GFXID_SCENE].set_number = 0;
  layouts[GFXID_COLOR_2D].set_number = 1;

  layouts[GFXID_SCENE].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, 1, sizeof(Scene)); // 2D.vert
  layouts[GFXID_COLOR_2D].add_binding(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Vector4)); // color.frag, text.frag

  for (u32 i = 0; i < layouts_count; i++) {
    gfx.create_set_layout(&layouts[i]);
    gfx.allocate_descriptor_set(&layouts[i]);
    gfx.init_layout_offsets(&layouts[i]);
  }

  Pipeline *shader = find_pipeline(PIPELINE_2D);
  shader->vertex_info = Vertex_XU::get_vertex_info();
  shader->set.add_layout(&layouts[GFXID_SCENE]);
  shader->set.add_push(SHADER_STAGE_VERTEX, sizeof(Object));

  shader->set.add_layout(&layouts[GFXID_COLOR_2D]);
}

void GFX::bind_pipeline(u32 id) {
  active_shader_id = id;

  Pipeline *pipeline = find_pipeline(id);
  Vulkan_Context::bind_pipeline(pipeline);
}

Descriptor GFX::descriptor(u32 gfx_layout_id) {
  return get_descriptor_set(&layouts[gfx_layout_id]);
}