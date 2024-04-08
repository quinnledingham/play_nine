/*
TODO:
- put all descriptors and buffers in vulkan info in a way that is easy to clean up
*/

struct Vulkan_Validation_Layers {
	const char *data[1] = { "VK_LAYER_KHRONOS_validation" };
	const u32 count = ARRAY_COUNT(data);

#ifdef DEBUG
	const bool8 enable = true; // it really does slow the performance down a lot.
#else
	const bool8 enable = false;
#endif // DEBUG
};

struct Vulkan_Queue_Family_Indices {
	bool8 graphics_family_found;
	bool8 present_family_found;

	u32 graphics_family;
	u32 present_family;

	static const u32 unique_families = 2;
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
    u32 offset; // where to enter new bytes
	void *data; // if the memory is mapped
};

struct Vulkan_Info {
	const char *device_extensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	
	// on top of the extensions that you need to make the surface
	const char *extra_instance_extensions[1] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	u32 current_frame; // which frame to fill ie. MAX_FRAMES_IN_FLIGHT = 2 either 0 or 1

	s32 window_width;
	s32 window_height;
	bool8 framebuffer_resized = false;
	bool8 minimized;

	Vulkan_Validation_Layers validation_layers;
	Vulkan_Swap_Chain_Support_Details swap_chain_support_details;

	VkDebugUtilsMessengerEXT debug_messenger;

	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkSampleCountFlagBits msaa_samples;
	VkPhysicalDeviceProperties physical_device_properties;
	VkDevice device;
	VkSurfaceKHR surface;

	VkRenderPass render_pass;         // general render pass for a pipeline (also pipeline layout)
	
	VkDeviceSize uniform_buffer_min_alignment;

	VkQueue graphics_queue;
	VkQueue present_queue;

	VkCommandPool command_pool;
	VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];
	u32 active_command_buffer_index;
	VkPipelineLayout pipeline_layout; // set this to the currently bounded layout

	// swap_chain
	VkSwapchainKHR swap_chains[1];
	Arr<VkImage> swap_chain_images;
	u32 image_index;                            // set at the start of the frame for the current frame
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;
	Arr<VkImageView> swap_chain_image_views;
	Arr<VkFramebuffer> swap_chain_framebuffers;

	// sync
	VkSemaphore image_available_semaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore render_finished_semaphore[MAX_FRAMES_IN_FLIGHT];
	VkFence in_flight_fence[MAX_FRAMES_IN_FLIGHT];

	// Buffers	
	Vulkan_Buffer static_buffer;
	Vulkan_Buffer static_uniform_buffer;
	Vulkan_Buffer dynamic_uniform_buffer;

	// Depth Buffer
	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	// MSAA
	VkImage color_image;
	VkDeviceMemory color_image_memory;
	VkImageView color_image_view;

	// Presentation
	VkCommandBufferBeginInfo begin_info;
	VkClearValue clear_values[2];

	VkPipelineStageFlags wait_stages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkRenderPassBeginInfo render_pass_info;
	VkSubmitInfo submit_info;
	VkPresentInfoKHR present_info;

	// Shaders
	u32 shader_handles[10];
	u32 shader_count;
	Shader *active_shader;

	// Descriptor_Sets
	VkDescriptorPool descriptor_pool;
};

inline VkCommandBuffer
vulkan_active_cmd_buffer(Vulkan_Info *info) {
	return info->command_buffers[info->current_frame];
}

struct Vulkan_Texture {
	VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

	VkDeviceMemory image_memory; // Could put in vulkan info and save it

	VkImage image;	 		// similar to VkBuffer
	VkImageView image_view; // provides more info about the image
	VkSampler sampler;      // allows the shader to sample the image

	u32 index;
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

global Vulkan_Info vulkan_info = {};