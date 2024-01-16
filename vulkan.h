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

// info to setup a graphics pipeline
struct Vulkan_Graphics_Pipeline {
	File vert; // compiled shaders
	File frag;
	VkVertexInputBindingDescription binding_description;
	VkVertexInputAttributeDescription attribute_descriptions[3];
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
	VkRenderPass render_pass;

	VkDeviceSize uniform_buffer_min_alignment;

	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;

	VkQueue graphics_queue;
	VkQueue present_queue;

	VkCommandPool command_pool;
	Arr<VkCommandBuffer> command_buffers;
	VkCommandBuffer command_buffer;             // set at the start of the frame for the current frame

	// swap_chain
	VkSwapchainKHR swap_chains[1];
	Arr<VkImage> swap_chain_images;
	u32 image_index;                            // set at the start of the frame for the current frame
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;
	Arr<VkImageView> swap_chain_image_views;
	Arr<VkFramebuffer> swap_chain_framebuffers;

	// sync
	Arr<VkSemaphore> image_available_semaphore;
	Arr<VkSemaphore> render_finished_semaphore;
	VkFence in_flight_fence[MAX_FRAMES_IN_FLIGHT];

	// Buffers	
    VkBuffer combined_buffer;
    VkDeviceMemory combined_buffer_memory;
    u32 combined_buffer_offset; // where to enter new bytes
	
	// Images
	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	// Presentation
	VkCommandBufferBeginInfo begin_info;
	VkClearValue clear_values[2];
	//VkViewport viewport;
	//VkRect2D scissor;

	VkPipelineStageFlags wait_stages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkRenderPassBeginInfo render_pass_info;
	VkSubmitInfo submit_info;
	VkPresentInfoKHR present_info;
};

global Vulkan_Info vulkan_info;

struct Vulkan_Texture {
	VkImage image;
	static const VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
	VkDeviceMemory image_memory;
	VkImageView image_view;
	VkSampler sampler;
};

struct Vulkan_Descriptor_Set {
	VkDescriptorSetLayout layout;
	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_sets[vulkan_info.MAX_FRAMES_IN_FLIGHT];
};

struct Vulkan_Mesh {
    u32 vertices_offset;
    u32 indices_offset;
    
    u32 uniform_offsets[vulkan_info.MAX_FRAMES_IN_FLIGHT];
    u32 uniform_size; // size of the individual uniforms
};
