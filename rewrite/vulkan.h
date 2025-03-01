struct Vulkan_Validation_Layers {
	const char *data[1] = { "VK_LAYER_KHRONOS_validation" };
	const u32 count = ARRAY_COUNT(data);

#ifdef DEBUG
	const bool8 enable = true; // it really does slow the performance down a lot.
#else
	const bool8 enable = false;
#endif // DEBUG
};

struct Vulkan_Version {
	u32 variant;
	u32 major;
	u32 minor;
	u32 patch;
};

struct Vulkan_Queue_Family {
	bool8 found;
	u32 index;
};

struct Vulkan_Queue_Family_Indices {
	union {
		struct {
			Vulkan_Queue_Family graphics_and_compute_family;
			Vulkan_Queue_Family present_family;
		};
		Vulkan_Queue_Family queue_families[2];
	};
};

struct Vulkan_Swap_Chain_Support_Details {
	VkSurfaceCapabilitiesKHR capabilities;
	
	VkSurfaceFormatKHR *formats;
	u32 formats_count;

	VkPresentModeKHR *present_modes;
	u32 present_modes_count;
};

struct Vulkan_Frame {
	u32 dynamic_offset_start;
	u32 dynamic_offset_end;
	
	union {
		struct {
			VkCommandBuffer command_buffer;
			VkCommandBuffer compute_command_buffer;
		};
		VkCommandBuffer command_buffers[2];
	};
	
	// sync
	union {
		struct {
			VkSemaphore image_available_semaphore;
			VkSemaphore render_finished_semaphore;
			VkSemaphore compute_finished_semaphore;
		};
		VkSemaphore semaphores[3];
	};

	union {
		struct {
			VkFence in_flight_fence;
			VkFence compute_in_flight_fence;
		};
		VkFence fences[2];
	};
};

struct Vulkan_Buffer {
	VkBuffer handle;
	VkDeviceMemory memory;
	VkDeviceSize size;
	
	u32 offset; // where to enter new bytes
	void *data; // if the memory is mapped
};

struct Vulkan_Texture {
	VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
	VkDeviceMemory image_memory; // Could put in vulkan info and save it
	VkImage image;	 		         // similar to VkBuffer
	VkImageView image_view;      // provides more info about the image
	VkSampler sampler;           // allows the shader to sample the image
};

#define MAX_FRAMES_IN_FLIGHT 2

struct Vulkan_Debug {
	u32 allocated_descriptors_uniform_buffer;
	u32 allocated_descriptors_sampler;
	u32 allocated_descriptors_storage_buffer;
};

struct Vulkan_Context {
	// Extensions
	const char *device_extensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
		
	// on top of the extensions that you need to make the surface
	const char *extra_instance_extensions[1] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	const char **instance_extensions;
	u32 instance_extensions_count;

	// Debug
  Vulkan_Validation_Layers validation_layers;
  VkDebugUtilsMessengerEXT debug_messenger;

	// State
  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	VkDevice device;

	Vulkan_Queue_Family_Indices queue_family_indices;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkQueue compute_queue;
	
	Vulkan_Frame frames[MAX_FRAMES_IN_FLIGHT];
	
	VkDescriptorPool descriptor_pool;
	VkCommandPool command_pool;

	bool8 swap_chain_created;
	VkSwapchainKHR swap_chains[1];

	VkExtent2D draw_extent;
	VkExtent2D swap_chain_extent; // size of window, size of swap_chain_images, the value returned by vulkan_choose_swap_extent
	
	Arr<Vulkan_Texture> swap_chain_textures;
	Arr<Vulkan_Texture> draw_textures; // where the frame gets drawn before swap chain buffer

	Vulkan_Texture depth_texture; // Depth Buffer
	Vulkan_Texture color_texture; // MSAA

	VkRenderPass draw_render_pass;
	VkRenderPass present_render_pass;

	Arr<VkFramebuffer> draw_framebuffers;
	Arr<VkFramebuffer> swap_chain_framebuffers; // framebuffers for the swap_chain

	VkSampleCountFlagBits msaa_samples;

	VkCommandBuffer *active_command_buffer;

	// Presentation
	VkClearValue clear_values[2];
	VkPipelineStageFlags wait_stages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	u32 current_frame;
	bool8 recording_frame = FALSE;

	//u32 current_frame; // which frame to fill ie. MAX_FRAMES_IN_FLIGHT = 2 either 0 or 1
	// Set at the start of the frame for the current frame.
 	// What frame buffer that should be used for that frame.
	u32 image_index; 

	VkDeviceSize uniform_buffer_min_alignment;

#ifdef DEBUG
	Vulkan_Debug debug;
#endif // DEBUG

	// Buffers
	union {
		struct {
			Vulkan_Buffer static_buffer;
			Vulkan_Buffer dynamic_buffer;
			Vulkan_Buffer static_uniform_buffer;
			Vulkan_Buffer dynamic_uniform_buffer;

			Vulkan_Buffer storage_buffer;
			Vulkan_Buffer triangle_buffer;
		};
		Vulkan_Buffer buffers[6];
	};

	// Functions
  s32 sdl_init(SDL_Window *window);
  void cleanup();
  void create_swap_chain(Vector2_s32 window_dim, bool8 vsync);
  void create_frame_resources();

	s32 sdl_load_instance_extensions(SDL_Window *sdl_window);
	bool8 create_instance();
	void setup_debug_messenger();
	bool8 pick_physical_device();
	void create_logical_device();
	void create_sync_objects();
	void create_command_pool();
	void create_command_buffers(VkCommandBuffer *command_buffers, u32 num_of_buffers);
	void create_descriptor_pool();
	void create_buffer(Vulkan_Buffer *buffer, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	void cleanup_swap_chain();
	void destroy_texture(void *gpu_handle);
	VkSampleCountFlagBits get_max_usable_sample_count();
	//void destroy_texture(Bitmap *bitmap);
	void create_render_image_views(Vector2_s32 dim);
	void create_sampler(VkSampler *sampler, u32 texture_parameters, u32 mip_levels);
	bool8 create_draw_render_pass(bool8 resolution_scaling, bool8 anti_aliasing);
	bool8 create_present_render_pass();
	void create_depth_resources(Vector2_s32 dim);
	void create_color_resources(Vector2_s32 dim);
	bool8 create_draw_framebuffers(bool8 resolution_scaling, bool8 anti_aliasing);
	bool8 create_swap_chain_framebuffers();
	bool8 start_frame();
	void end_frame(bool8 resolution_scaling, bool8 window_resized);
	s32 create_graphics_pipeline(Shader *shader, VkRenderPass render_pass);

	void device_wait_idle() { vkDeviceWaitIdle(device); }
	void destroy_render_pass(VkRenderPass pass) { vkDestroyRenderPass(device, pass, nullptr); }
	void clear_color(Vector4 color) { clear_values[0].color = {{color.r, color.g, color.b, color.a}}; }

	void set_viewport(u32 window_width, u32 window_height);
	void set_scissor(s32 x, s32 y, u32 width, u32 height);
	void bind_pipeline(Pipeline *pipeline);

	void create_set_layout(GFX_Layout *layout);
	void allocate_descriptor_set(GFX_Layout *layout);
	void init_layout_offsets(GFX_Layout *layout); // @TODO add Bitmap *bitmap back
	void init_ubos(VkDescriptorSet *sets, GFX_Layout_Binding *layout_binding, u32 num_of_sets, u32 offsets[64]);

	void copy_buffer(Vulkan_Buffer src_buffer, Vulkan_Buffer dest_buffer, VkDeviceSize size, u32 src_offset, u32 dest_offset);
	void create_buffer(Vulkan_Buffer *buffer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	void update_buffer(Vulkan_Buffer *buffer, void *in_data, u32 in_data_size, u32 offset);

	void init_mesh(Mesh *mesh);
	void draw_mesh(Mesh *mesh);
};

struct Vulkan_Pipeline {

};

struct Vulkan_Mesh {
	u32 vertices_offset;
	u32 indices_offset;

	u32 uniform_offsets[MAX_FRAMES_IN_FLIGHT];
	u32 uniform_size; // size of the individual uniforms
};

#define VULKAN_STATIC_BUFFER_SIZE 100000
#define VULKAN_STATIC_UNIFORM_BUFFER_SIZE 1000000
#define VULKAN_DYNAMIC_UNIFORM_BUFFER_SIZE 1000000

void vulkan_log_error(const char *msg, ...) {
  print_char_array(OUTPUT_ERROR, "ERROR: (vulkan)");

  OUTPUT_LIST(OUTPUT_ERROR, msg);
}