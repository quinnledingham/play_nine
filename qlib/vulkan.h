struct Vulkan_Validation_Layers {
	const char *data[1] = { "VK_LAYER_KHRONOS_validation" };
	const u32 count = ARRAY_COUNT(data);

#ifdef DEBUG
	const bool8 enable = true; // it really does slow the performance down a lot.
#else
	const bool8 enable = false;
#endif // DEBUG
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

struct Vulkan_Texture {
	VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
	VkDeviceMemory image_memory; // Could put in vulkan info and save it
	VkImage image;	 		    // similar to VkBuffer
	VkImageView image_view; // provides more info about the image
	VkSampler sampler;      // allows the shader to sample the image
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

struct Vulkan_Info {
	const char *device_extensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	
	// on top of the extensions that you need to make the surface
	const char *extra_instance_extensions[1] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	bool8 minimized;

	Vulkan_Validation_Layers validation_layers;
	Vulkan_Swap_Chain_Support_Details swap_chain_support_details;

	VkDebugUtilsMessengerEXT debug_messenger;

	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	VkDevice device;
	VkSurfaceKHR surface;

	VkRenderPass draw_render_pass;
	VkRenderPass present_render_pass;
	
	VkDeviceSize uniform_buffer_min_alignment;
	VkSampleCountFlagBits msaa_samples;

	Vulkan_Queue_Family_Indices indices;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkQueue compute_queue;

	VkDescriptorPool descriptor_pool;
	VkCommandPool command_pool;
	
	VkCommandBuffer *active_command_buffer;
	VkPipelineLayout pipeline_layout; // set this to the currently bounded layout
	Shader *active_shader;
		
	// swap_chain
	VkSwapchainKHR swap_chains[1];
	VkExtent2D swap_chain_extent; // size of window, size of swap_chain_images, the value returned by vulkan_choose_swap_extent
	
	Arr<Vulkan_Texture> draw_textures; // where the frame gets drawn before swap chain buffer
	Arr<Vulkan_Texture> swap_chain_textures;
	
	Arr<VkFramebuffer> draw_framebuffers;
	Arr<VkFramebuffer> swap_chain_framebuffers; // framebuffers for the swap_chain
	
	//u32 current_frame; // which frame to fill ie. MAX_FRAMES_IN_FLIGHT = 2 either 0 or 1
	// Set at the start of the frame for the current frame.
 	// What frame buffer that should be used for that frame.
	u32 image_index; 

	Vulkan_Frame frames[MAX_FRAMES_IN_FLIGHT];

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
	
	Vulkan_Texture depth_texture; // Depth Buffer
	Vulkan_Texture color_texture; // MSAA

	// Presentation
	VkClearValue clear_values[2];
	VkPipelineStageFlags wait_stages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkCommandBufferBeginInfo begin_info;
	VkRenderPassBeginInfo draw_render_pass_info;
	VkRenderPassBeginInfo present_render_pass_info;
	VkSubmitInfo submit_info;
	VkSubmitInfo compute_submit_info;
	VkPresentInfoKHR present_info;

	u32 allocated_descriptors_uniform_buffer;
	u32 allocated_descriptors_sampler;
	u32 allocated_descriptors_storage_buffer;
};

struct Vulkan_Mesh {
    u32 vertices_offset;
    u32 indices_offset;
    
    u32 uniform_offsets[MAX_FRAMES_IN_FLIGHT];
    u32 uniform_size; // size of the individual uniforms
};

struct Vulkan_Version {
	u32 variant;
	u32 major;
	u32 minor;
	u32 patch;
};

#define VK_CMD(i) *i.active_command_buffer

#define VULKAN_STATIC_BUFFER_SIZE 100000
#define VULKAN_STATIC_UNIFORM_BUFFER_SIZE 1000000
#define VULKAN_DYNAMIC_UNIFORM_BUFFER_SIZE 100000

global Vulkan_Info vulkan_info = {};
Render_Pipeline present_pipeline;
