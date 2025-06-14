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

struct Vulkan_Vendor_IDs {
	const u32 intel = 0x8086;
	const u32 nvidia = 4318;
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

struct Vulkan_Texture {
	VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
	VkDeviceMemory image_memory; // Could put in vulkan info and save it
	VkImage image;	 		         // similar to VkBuffer
	VkImageView image_view;      // provides more info about the image
	VkSampler sampler;           // allows the shader to sample the image
};

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

	Vulkan_Vendor_IDs vendor_ids;
	
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

};

#define VK_CMD *vk_ctx.active_command_buffer

struct Vulkan_Pipeline {

};

struct Vulkan_Mesh {
	u32 vertices_offset;
	u32 indices_offset;

	u32 uniform_offsets[MAX_FRAMES_IN_FLIGHT];
	u32 uniform_size; // size of the individual uniforms

	Vulkan_Buffer *buffer;
};

#define VULKAN_STATIC_BUFFER_SIZE 10000000
#define VULKAN_STATIC_UNIFORM_BUFFER_SIZE 1000000
#define VULKAN_DYNAMIC_UNIFORM_BUFFER_SIZE 1000000

void vulkan_log(const char *msg, ...) {
  print_char_array(OUTPUT_DEFAULT, "(vulkan) ");

  OUTPUT_LIST(OUTPUT_ERROR, msg);
}

void vulkan_log_error(const char *msg, ...) {
  print_char_array(OUTPUT_ERROR, "ERROR: (vulkan) ");

  OUTPUT_LIST(OUTPUT_ERROR, msg);
}

internal void vulkan_setup_layout(GFX_Layout *layout);

inline void
vulkan_split_buffer_over_frames(Vulkan_Buffer *buffer, Vulkan_Frame *frames, u32 frames_count) {
	u32 segment_size = (u32)buffer->size / frames_count;
	u32 offset = 0;
	for (u32 i = 0; i < frames_count; i++) {
		Vulkan_Frame *frame = &frames[i];
		// @TODO make this work for other dynamic buffers in frame
		frame->dynamic_buffer_segment.buffer = buffer;
		frame->dynamic_buffer_segment.start = offset;
		frame->dynamic_buffer_segment.end = offset + segment_size;
		offset += segment_size;
	}
}

inline void
vulkan_set_buffer_segment(Vulkan_Buffer_Segment *seg) {
	seg->buffer->offset = seg->start;
}

inline void
vulkan_check_buffer_segment(Vulkan_Buffer_Segment *seg) {
	if (seg->end < seg->buffer->offset) {
		vulkan_log_error("vulkan_end_frame() used to much space on the dynamic buffer\n");
		ASSERT(0);
	}
}

