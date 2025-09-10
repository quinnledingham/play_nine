struct Vulkan_Version {
	u32 variant;
	u32 major;
	u32 minor;
	u32 patch;
};

struct Vulkan_Vendor_IDs {
	static const u32 intel = 0x8086;
	static const u32 nvidia = 4318;
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

struct Vulkan_Buffer {
  VkBuffer handle;
  VkDeviceMemory memory;
  VkDeviceSize size;
  
  u32 offset; // where to enter new bytes
  void *data; // if the memory is mapped
};

struct Vulkan_Buffer_Segment{
	Vulkan_Buffer *buffer;
	u32 start;
	u32 end;
};

struct Vulkan_Frame {
	Vulkan_Buffer_Segment dynamic_buffer_segment;

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

/*

	TEXTURE DEFINITION FOR ASSETS

*/

struct Vulkan_Texture {
  VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
  VkDeviceMemory memory;  // Could put in vulkan info and save it
  VkImage image;      // similar to VkBuffer
  VkImageView image_view; // provides more info about the image
  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkSampler sampler;      // allows the shader to sample the image
};

struct Bitmap {
  u8 *data; 
  union {
      struct {
          s32 width;
          s32 height;
      };
      Vector2_s32 dim;
  };
  s32 pitch;
  s32 channels;
  u32 mip_levels = 1;
};

struct Texture : Vulkan_Texture {
  u8 *data; 
  union {
      struct {
          s32 width;
          s32 height;
      };
      Vector2_s32 dim;
  };
  s32 pitch;
  s32 channels;
  u32 mip_levels = 1;
};

struct Vulkan_Context {
  // Debug
  VkDebugUtilsMessengerEXT debug_messenger;
  const bool8 validation_layers_enabled = true;
  const char *validation_layers[1] = {
    "VK_LAYER_KHRONOS_validation"
  };
    
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

	// State
	static const u32 max_frames_in_flight = 2;

  VkInstance instance;

  VkSurfaceKHR surface;
  VkSurfaceCapabilitiesKHR surface_capabilities;
	Arr<VkSurfaceFormatKHR> formats;
	Arr<VkPresentModeKHR> present_modes;

  VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;

	VkDevice device;

	Vulkan_Queue_Family_Indices queue_family_indices;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkQueue compute_queue;

	Vulkan_Frame frames[Vulkan_Context::max_frames_in_flight];

	VkCommandPool command_pool;
	VkDescriptorPool descriptor_pool;

	// Swap Chain
	bool8 swap_chain_created;
	VkSwapchainKHR swap_chains[1];
	VkExtent2D draw_extent;
	VkExtent2D swap_chain_extent; // size of window, size of swap_chain_images, the value returned by vulkan_choose_swap_extent
	Arr<Texture> swap_chain_textures;
	Arr<Texture> draw_textures;
	Arr<VkFramebuffer> draw_framebuffers;
	Arr<VkFramebuffer> swap_chain_framebuffers; // framebuffers for the swap_chain

	VkRenderPass draw_render_pass;
	VkRenderPass present_render_pass;

	Texture depth_texture; // Depth Buffer
	Texture color_texture; // MSAA

	Vector2_s32 window_dim;
	Vector2_s32 resolution;
	VkSampleCountFlagBits msaa_samples;
	VkDeviceSize uniform_buffer_min_alignment;

	u32 current_frame;
	u32 image_index; 
	VkCommandBuffer *active_command_buffer;
	VkClearValue clear_values[2];
	VkPipelineStageFlags wait_stages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  u32 active_shader_id;

	// Flags
	bool8 vsync;
	bool8 resolution_scaling;
	bool8 anti_aliasing;
	bool8 recording_frame = FALSE;
	bool8 window_resized;

  GFX_Layout *layouts;
  u32 layouts_count;

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
};

#define VULKAN_STATIC_BUFFER_SIZE 10000000
#define VULKAN_STATIC_UNIFORM_BUFFER_SIZE 1000000
#define VULKAN_DYNAMIC_UNIFORM_BUFFER_SIZE 1000000

#define vulkan_log(fmt, ...) SDL_Log("(vulkan) " fmt, ##__VA_ARGS__)
#define vulkan_log_error(fmt, ...) SDL_LogError(GOLFO_VULKAN, "(vulkan) " fmt, ##__VA_ARGS__)
#define VK_CMD *vk_ctx.active_command_buffer

enum Texture_Parameters {
    TEXTURE_PARAMETERS_DEFAULT,
    TEXTURE_PARAMETERS_CHAR,
};

internal void vulkan_setup_layout(GFX_Layout *layout);

inline void
vulkan_set_buffer_segment(Vulkan_Buffer_Segment *seg) {
	seg->buffer->offset = seg->start;
}
