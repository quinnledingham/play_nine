struct Vulkan_Validation_Layers {
	const char *data[1] = { "VK_LAYER_KHRONOS_validation" };
	const u32 count = ARRAY_COUNT(data);

#ifdef DEBUG
	const bool8 enable = true;
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

struct Vulkan_Info {
	const char *device_extensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	static const u32 MAX_FRAMES_IN_FLIGHT = 2;
	u32 current_frame;

	s32 window_width;
	s32 window_height;
	bool8 framebuffer_resized = false;
	bool8 minimized;

	Vulkan_Validation_Layers validation_layers;
	Vulkan_Swap_Chain_Support_Details swap_chain_support_details;

	VkDebugUtilsMessengerEXT debug_messenger;

	VkInstance instance;
	VkPhysicalDevice physical_device;
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
	//VkCommandBuffer command_buffer;   // set at the start of the frame for the current frame
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
    VkBuffer static_buffer;
    VkDeviceMemory static_buffer_memory;
    u32 static_buffer_offset; // where to enter new bytes
	
	VkBuffer uniform_buffer;
	VkDeviceMemory uniform_buffer_memory;
	u32 dymanic_uniforms_offset; // where to go to update the dynamic uniforms
	u32 uniform_buffer_offset;
	void *uniform_data;

	// Depth Buffer
	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	// Presentation
	VkCommandBufferBeginInfo begin_info;
	VkClearValue clear_values[2];

	VkPipelineStageFlags wait_stages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkRenderPassBeginInfo render_pass_info;
	VkSubmitInfo submit_info;
	VkPresentInfoKHR present_info;
};

inline VkCommandBuffer
vulkan_active_cmd_buffer(Vulkan_Info *info) {
	return info->command_buffers[info->current_frame];
}

global Vulkan_Info vulkan_info;

struct Vulkan_Texture {
	VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

	VkDeviceMemory image_memory; // Could put in vulkan info an save it

	VkImage image;
	VkImageView image_view;
	VkSampler sampler;
};

struct Vulkan_Descriptor_Set {
	VkDescriptorSet descriptor_sets[vulkan_info.MAX_FRAMES_IN_FLIGHT];
};

struct Vulkan_Mesh {
    u32 vertices_offset;
    u32 indices_offset;
    
    u32 uniform_offsets[vulkan_info.MAX_FRAMES_IN_FLIGHT];
    u32 uniform_size; // size of the individual uniforms
};
