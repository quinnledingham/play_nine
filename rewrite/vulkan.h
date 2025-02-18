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

struct Vulkan_Context {
  const char *device_extensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	
	// on top of the extensions that you need to make the surface
	const char *extra_instance_extensions[1] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

  Vulkan_Validation_Layers validation_layers;
  VkDebugUtilsMessengerEXT debug_messenger;

  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	VkDevice device;

  Vulkan_Queue_Family_Indices queue_family_indices;
};
Vulkan_Context vulkan_context = {};

// Functions
s32 vulkan_sdl_init(SDL_Window *sdl_window);