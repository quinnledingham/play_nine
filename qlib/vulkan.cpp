//
// Vulkan Debug
//

internal VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
	print("(vulkan) %s\n", callback_data->pMessage);
	return VK_FALSE;
}

internal void 
vulkan_populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info) {
    create_info = {};
    create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = vulkan_debug_callback;
}

internal VkResult 
vulkan_create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, create_info, allocator, debug_messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

internal void 
vulkan_destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, allocator);
    }
}

internal void
vulkan_setup_debug_messenger(Vulkan_Info *info) {
	if (!info->validation_layers.enable)
		return;

	VkDebugUtilsMessengerCreateInfoEXT create_info;
    vulkan_populate_debug_messenger_create_info(create_info);

	if (vulkan_create_debug_utils_messenger_ext(info->instance, &create_info, nullptr, &info->debug_messenger) != VK_SUCCESS) {
		logprint("vulkan_create_debug_messenger()", "failed to set up debug messenger\n");
	}
}

internal bool8
vulkan_check_validation_layer_support(Vulkan_Validation_Layers validation_layers) {
    u32 layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    VkLayerProperties *available_layers = ARRAY_MALLOC(VkLayerProperties, layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

    bool8 all_layers_found = true;
    for (u32 validation_index = 0; validation_index < validation_layers.count; validation_index++) {
    	bool8 layer_found = false;

    	for (u32 available_index = 0; available_index < layer_count; available_index++) {
    		if (equal(validation_layers.data[validation_index], available_layers[available_index].layerName)) {
    			layer_found = true;
    			break;
    		}
    	}

    	if (!layer_found) {
    		all_layers_found = false;
    		break;
    	}
    }

    platform_free(available_layers);

    return all_layers_found;
}

//
// Initialization
//

internal bool8
vulkan_create_instance(Vulkan_Info *vulkan_info, const char **instance_extensions, u32 instance_extensions_count) {
  VkInstance instance;

  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Hello Triangle";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledExtensionCount = instance_extensions_count;
	create_info.ppEnabledExtensionNames = (const char *const *)instance_extensions;

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
	if (vulkan_info->validation_layers.enable) {
    create_info.enabledLayerCount = vulkan_info->validation_layers.count;
    create_info.ppEnabledLayerNames = vulkan_info->validation_layers.data;

		vulkan_populate_debug_messenger_create_info(debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	} else {
		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;
	}

	if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
	    logprint("main()", "failed to create vulkan instance");
	    return 1;
	}

	vulkan_info->instance = instance;
	return 0;
}


internal Vulkan_Queue_Family_Indices
vulkan_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
	Vulkan_Queue_Family_Indices indices;

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	VkQueueFamilyProperties *queue_families = ARRAY_MALLOC(VkQueueFamilyProperties, queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

	for (u32 queue_index = 0; queue_index < queue_family_count; queue_index++) {
		if ((queue_families[queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue_families[queue_index].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
			indices.graphics_and_compute_family.found = true;
			indices.graphics_and_compute_family.index = queue_index;
		}

		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_index, surface, &present_support);
		if (present_support) {
			indices.present_family.found = true;
			indices.present_family.index = queue_index;
		}
	}

	platform_free(queue_families);

	return indices;
}

internal bool8
vulkan_check_device_extension_support(VkPhysicalDevice device, const char **device_extensions, u32 device_extensions_count) {
	u32 available_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &available_count, nullptr);

    VkExtensionProperties *available_extensions = ARRAY_MALLOC(VkExtensionProperties, available_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &available_count, available_extensions);

    bool8 all_extensions_found = true;
    for (u32 required_index = 0; required_index < device_extensions_count; required_index++) {
    	bool8 extension_found = false;
    	for (u32 available_index = 0; available_index < available_count; available_index++) {
    		if (equal(available_extensions[available_index].extensionName, device_extensions[required_index])) {
    			extension_found = true;
    			break;
    		}
    	}

    	if (!extension_found) {
    		all_extensions_found = false;
    		break;
    	}
    }
	
	platform_free(available_extensions);

    return all_extensions_found;
}

internal Vulkan_Swap_Chain_Support_Details
vulkan_query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
	Vulkan_Swap_Chain_Support_Details details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formats_count, nullptr);
	if (details.formats_count != 0) {
		details.formats = ARRAY_MALLOC(VkSurfaceFormatKHR, details.formats_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formats_count, details.formats);
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.present_modes_count, nullptr);
	if (details.present_modes_count != 0) {
		details.present_modes = ARRAY_MALLOC(VkPresentModeKHR, details.present_modes_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.present_modes_count, details.present_modes);
	}

	return details;
}

internal bool8
vulkan_is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface, const char **device_extensions, u32 device_extensions_count) {
	bool8 extensions_supported = vulkan_check_device_extension_support(device, device_extensions, device_extensions_count);
	bool8 swap_chain_adequate = false;
	if (extensions_supported) {
		Vulkan_Swap_Chain_Support_Details swap_chain_support = vulkan_query_swap_chain_support(device, surface);
		swap_chain_adequate = swap_chain_support.formats_count && swap_chain_support.present_modes_count;
		platform_free(swap_chain_support.formats);
		platform_free(swap_chain_support.present_modes);
	}

	Vulkan_Queue_Family_Indices indices = vulkan_find_queue_families(device, surface);
	VkPhysicalDeviceProperties device_properties = {};
	VkPhysicalDeviceFeatures device_features = {};
	vkGetPhysicalDeviceProperties(device, &device_properties);
	vkGetPhysicalDeviceFeatures(device, &device_features);

	logprint("vulkan_is_device_suitable()", "Vulkan Physical Device Limits:\nMax Descriptor Set Samplers: %u\nMax Descriptor Set Uniform Buffers: %u\nMax Uniform Buffer Range: %u\n", device_properties.limits.maxDescriptorSetSamplers, device_properties.limits.maxDescriptorSetUniformBuffers, device_properties.limits.maxUniformBufferRange); 
	return indices.graphics_and_compute_family.found && extensions_supported && swap_chain_adequate && device_features.samplerAnisotropy && device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

internal VkSampleCountFlagBits
vulkan_get_max_usable_sample_count() {
    VkSampleCountFlags counts = vulkan_info.physical_device_properties.limits.framebufferColorSampleCounts & 
                                vulkan_info.physical_device_properties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT)  { return VK_SAMPLE_COUNT_8_BIT;  }
    if (counts & VK_SAMPLE_COUNT_4_BIT)  { return VK_SAMPLE_COUNT_4_BIT;  }
    if (counts & VK_SAMPLE_COUNT_2_BIT)  { return VK_SAMPLE_COUNT_2_BIT;  }

    return VK_SAMPLE_COUNT_1_BIT;
}

inline Vulkan_Version
vulkan_get_api_version(u32 api_version) {
	Vulkan_Version version = {};
	version.variant = VK_API_VERSION_VARIANT(api_version);
	version.major   = VK_API_VERSION_MAJOR(api_version);
	version.minor   = VK_API_VERSION_MINOR(api_version);
	version.patch   = VK_API_VERSION_PATCH(api_version);
	return version;
}

inline Vulkan_Version
vulkan_get_nvidia_driver_version(u32 api_version) {
	Vulkan_Version version = {};
	version.variant = 0;
	version.major = ((u32)api_version >> 22) & 0x000003FF;
	version.minor = ((u32)api_version >> 12) & 0x000003FF;
	version.patch = ((u32)api_version >>  0) & 0x00000FFF;
	return version;
}

inline void
vulkan_print_version(const char *name, Vulkan_Version version) {
	if (version.variant)
		print("%s: %d.%d.%d.%d\n", name, version.variant, version.major, version.minor, version.patch);
	else
		print("%s: %d.%d.%d\n", name, version.major, version.minor, version.patch);
}

internal bool8
vulkan_pick_physical_device(Vulkan_Info *info) {
	info->physical_device = VK_NULL_HANDLE;

	u32 device_count = 0;
	vkEnumeratePhysicalDevices(info->instance, &device_count, nullptr);

	if (device_count == 0) {
		logprint("vulkan_pick_physical_device()", "failed to find GPUs with Vulkan support");
		return 1;
	}

	logprint("vulkan_pick_physical_deivce()", "Number of Physical Devies = %d\n", device_count);
	
	VkPhysicalDevice *devices = ARRAY_MALLOC(VkPhysicalDevice, device_count);
	vkEnumeratePhysicalDevices(info->instance, &device_count, devices);

	// print all available devies
	print("vulkan_pick_physical_device(): Avaiable Physical Devices:\n");
	for (u32 device_index = 0; device_index < device_count; device_index++) {
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(devices[device_index], &device_properties);
		
		Vulkan_Version api_version = vulkan_get_api_version(device_properties.apiVersion);
		Vulkan_Version driver_version = vulkan_get_nvidia_driver_version(device_properties.driverVersion);
		
		print("Vulkan Physical Device %d:\n", device_index);
		vulkan_print_version("  API Version", api_version);
		vulkan_print_version("  Driver Version", driver_version);
		print("  Device Name %s\n", device_properties.deviceName);
	}

	// pick suitable device
	for (u32 device_index = 0; device_index < device_count; device_index++) {
		if (vulkan_is_device_suitable(devices[device_index], info->surface, info->device_extensions, ARRAY_COUNT(info->device_extensions))) {
			info->physical_device = devices[device_index];
			print("(vulkan) Picking device %d\n", device_index);
			break;
		}
	}

	platform_free(devices);

	if (info->physical_device == VK_NULL_HANDLE) {
		logprint("vulkan_pick_physical_device()", "failed to find a suitable GPU\n");
		return 1;
	}

	vkGetPhysicalDeviceProperties(vulkan_info.physical_device, &vulkan_info.physical_device_properties);
	info->indices = vulkan_find_queue_families(vulkan_info.physical_device, info->surface);
	
	return 0;
}

internal void
vulkan_create_logical_device(Vulkan_Info *info) {
	// Specify the device queues we want
	VkDeviceQueueCreateInfo *queue_create_infos = ARRAY_MALLOC(VkDeviceQueueCreateInfo, ARRAY_COUNT(info->indices.queue_families));
	u32 *unique_queue_families = ARRAY_MALLOC(u32, ARRAY_COUNT(info->indices.queue_families));
	u32 unique_queue_families_index = 0;

	for (u32 i = 0; i < ARRAY_COUNT(info->indices.queue_families); i++) {
		bool8 unique = true;
		for (u32 j = 0; j < unique_queue_families_index; j++) {
			if (i != j && unique_queue_families[j] == info->indices.queue_families[i].index) {
				unique = false;
				break;
			}
		}
		if (unique) unique_queue_families[unique_queue_families_index++] = info->indices.queue_families[i].index;
	}
	
	float32 queue_priority = 1.0f;
	for (u32 queue_index = 0; queue_index < unique_queue_families_index; queue_index++) {
		VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = unique_queue_families[queue_index];
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos[queue_index] = queue_create_info;
	}

	// Features requested
	VkPhysicalDeviceFeatures device_features = {};
	device_features.samplerAnisotropy = VK_TRUE;
	device_features.sampleRateShading = VK_TRUE; // performance cost

	// Set up device
	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_create_infos;
	create_info.queueCreateInfoCount = unique_queue_families_index;
	create_info.pEnabledFeatures = &device_features;
	create_info.enabledExtensionCount = ARRAY_COUNT(info->device_extensions);
	create_info.ppEnabledExtensionNames = (const char *const *)info->device_extensions;

	if (info->validation_layers.enable) {
		create_info.enabledLayerCount = info->validation_layers.count;
		create_info.ppEnabledLayerNames = info->validation_layers.data;
	} else {
		create_info.enabledLayerCount = 0;
	}

	if (vkCreateDevice(info->physical_device, &create_info, nullptr, &info->device) != VK_SUCCESS) {
		logprint("vulkan_create_logical_device", "failed to create logical device\n");
	}

	// Create the queues
	vkGetDeviceQueue(info->device, info->indices.graphics_and_compute_family.index, 0, &info->graphics_queue);
	vkGetDeviceQueue(info->device, info->indices.present_family.index, 0, &info->present_queue);
	vkGetDeviceQueue(info->device, info->indices.graphics_and_compute_family.index, 0, &info->compute_queue);

	platform_free(queue_create_infos);
	platform_free(unique_queue_families);
}

// Where SRGB is turned on
internal VkSurfaceFormatKHR
vulkan_choose_swap_surface_format(VkSurfaceFormatKHR *formats, u32 count) {
	VkFormat vulkan_target_formats[2] = {
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_FORMAT_R8G8B8A8_SRGB
	}; 
	
/*	for (u32 i = 3; i < count; i++) {
		if ((formats[i].format == vulkan_target_formats[0] || formats[i].format == vulkan_target_formats[1])
		    && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return formats[i];
		}
	}*/

	logprint("(vulkan) choose_swap_surface_format", "Picking default format\n(vulkan) Other formats available:\n");
	for (u32 i = 0; i < count; i++) {
		print("(vulkan) format: %d, color space: %d\n", formats[i].format, formats[i].colorSpace);
	}

	return formats[0];
}

// VSYNC SETTINGS AREA
// VK_PRESENT_MODE_IMMEDIATE_KHR 
// VK_PRESENT_MODE_MAILBOX_KHR   (VSYNC)
// VK_PRESENT_MODE_FIFO_KHR      (VSYNC)
internal VkPresentModeKHR
vulkan_choose_swap_present_mode(VkPresentModeKHR *modes, u32 count) {
	if (render_context.vsync)
		return VK_PRESENT_MODE_FIFO_KHR;
	
	for (u32 i = 0; i < count; i++) {
		if (modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			return modes[i];
		}
	}
	
	return VK_PRESENT_MODE_FIFO_KHR; // this mode is required to be supported
}

// swap extent might get clamped
internal VkExtent2D
vulkan_choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities, u32 window_width, u32 window_height) {
	if (capabilities.currentExtent.width != 0xFFFFFFFF) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actual_extent = { window_width, window_height };
		actual_extent.width  = clamp(actual_extent.width,  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width);
		actual_extent.height = clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actual_extent;
	}
}

internal void
vulkan_create_swap_chain(Vulkan_Info *info, Vector2_s32 window_dim) {
	Vulkan_Swap_Chain_Support_Details swap_chain_support = vulkan_query_swap_chain_support(info->physical_device, info->surface);

	VkSurfaceFormatKHR surface_format = vulkan_choose_swap_surface_format(swap_chain_support.formats, swap_chain_support.formats_count);
	VkPresentModeKHR present_mode = vulkan_choose_swap_present_mode(swap_chain_support.present_modes, swap_chain_support.present_modes_count);
	VkExtent2D extent = vulkan_choose_swap_extent(swap_chain_support.capabilities, window_dim.width, window_dim.height);
	print("(vulkan) extent_w: %d, extent_h: %d\n", extent.width, extent.height);
	print("(vulkan) surface_format: %d, surface_color_space: %d\n", surface_format.format, surface_format.colorSpace);

	u32 image_count = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) {
		image_count = swap_chain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = info->surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	create_info.preTransform = swap_chain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;
	
	u32 queue_family_indices[2] = { info->indices.graphics_and_compute_family.index, info->indices.present_family.index };

	if (info->indices.graphics_and_compute_family.index != info->indices.present_family.index) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = ARRAY_COUNT(queue_family_indices);
		create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0; // optional
		create_info.pQueueFamilyIndices = nullptr; // optional
	}

	if (vkCreateSwapchainKHR(info->device, &create_info, nullptr, &info->swap_chains[0])) {
		logprint("vulkan_create_swap_chain()", "failed to create swap chain\n");
	}

	Arr<VkImage> swap_chain_images;
	swap_chain_images.resize(image_count);
	vkGetSwapchainImagesKHR(info->device, info->swap_chains[0], &image_count, swap_chain_images.get_data());

	info->swap_chain_extent = extent;
	
	info->swap_chain_textures.resize(image_count);
	for (u32 i = 0; i < info->swap_chain_textures.get_size(); i++) {
		info->swap_chain_textures[i].image_format = surface_format.format;
		info->swap_chain_textures[i].image = swap_chain_images[i];
	}
	
	platform_free(swap_chain_support.formats);
	platform_free(swap_chain_support.present_modes);
}

internal VkFormat
vulkan_find_supported_format(VkPhysicalDevice physical_device, VkFormat *candidates, u32 candidates_count, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (u32 i = 0; i < candidates_count; i++) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physical_device, candidates[i], &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
				return candidates[i];
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
		    return candidates[i];
		}
	}

	logprint("vulkan_find_supported_format()", "failed to find supported format\n");
	return {};
}

internal VkFormat
vulkan_find_depth_format(VkPhysicalDevice physical_device) {
	VkFormat candidates[3] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

	VkFormat format = vulkan_find_supported_format(physical_device, candidates, 3, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	return format;
}

internal bool8
vulkan_create_draw_render_pass(Vulkan_Info *info) {
	VkImageLayout final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	if (render_context.resolution_scaling)
		//final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		final_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

	VkAttachmentDescription color_attachment = {};
	color_attachment.format         = info->draw_textures[0].image_format;
	color_attachment.samples        = info->msaa_samples;
	color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	if (render_context.anti_aliasing)
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	else
		color_attachment.finalLayout = final_layout;
		//color_attachment.finalLayout  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
 
	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment = {};
	depth_attachment.format         = vulkan_find_depth_format(info->physical_device);
	depth_attachment.samples        = info->msaa_samples;
	depth_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription color_attachment_resolve = {};
	color_attachment_resolve.format         = info->draw_textures[0].image_format;
	color_attachment_resolve.samples        = VK_SAMPLE_COUNT_1_BIT;
	color_attachment_resolve.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_resolve.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_resolve.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_resolve.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_resolve.finalLayout    = final_layout;

	VkAttachmentReference color_attachment_resolve_ref = {};
	color_attachment_resolve_ref.attachment = 2;
	color_attachment_resolve_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount    = 1;
	subpass.pColorAttachments       = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;
	if (render_context.anti_aliasing)
  		subpass.pResolveAttachments = &color_attachment_resolve_ref;
	
	// Subpass dependencies
	VkSubpassDependency dependencies[1];
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	
	/*
	ALLOWS FOR SECOND RENDER PASS
	
	VkSubpassDependency dependencies[2];
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_NONE_KHR;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;							
	*/																								
	VkAttachmentDescription attachments[] = { color_attachment, depth_attachment, color_attachment_resolve };
	
	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	if (render_context.anti_aliasing)
		render_pass_info.attachmentCount = ARRAY_COUNT(attachments);
	else
		render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = attachments;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = ARRAY_COUNT(dependencies);
	render_pass_info.pDependencies = dependencies;

	if (vkCreateRenderPass(info->device, &render_pass_info, nullptr, &info->draw_render_pass) != VK_SUCCESS) {
		logprint("vulkan_create_render_pass()", "failed to create render pass\n");
		return 1;
	}

	return 0;
}

internal bool8
vulkan_create_present_render_pass() {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format         = vulkan_info.swap_chain_textures[0].image_format;
	color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount    = 1;
	subpass.pColorAttachments       = &color_attachment_ref;
	
	VkRenderPassCreateInfo render_pass_create_info = {};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &color_attachment;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;

	if (vkCreateRenderPass(vulkan_info.device, &render_pass_create_info, nullptr, &vulkan_info.present_render_pass) != VK_SUCCESS) {
		logprint("vulkan_create_render_pass()", "failed to create render pass\n");
		return 1;
	}

	return 0;
	
}

internal bool8
vulkan_create_draw_framebuffers(Vulkan_Info *info) {	
	info->draw_framebuffers.resize(info->swap_chain_textures.get_size());
	Arr<Vulkan_Texture> *textures;
	
	VkImageView attachments[3];
	VkFramebufferCreateInfo framebuffer_info = {};
	framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebuffer_info.renderPass      = vulkan_info.draw_render_pass;
  framebuffer_info.pAttachments    = attachments;
  framebuffer_info.attachmentCount = 3;
	if (render_context.resolution_scaling) {
  	textures = &info->draw_textures;
	  framebuffer_info.width           = render_context.resolution.width;
  	framebuffer_info.height          = render_context.resolution.height;
  } else {
  	textures = &info->swap_chain_textures;
		framebuffer_info.width           = info->swap_chain_extent.width;
		framebuffer_info.height          = info->swap_chain_extent.height;
  }
  framebuffer_info.layers          = 1;
  
	if (render_context.anti_aliasing) {
		attachments[0] = info->color_texture.image_view;
		attachments[1] = info->depth_texture.image_view;
	} else {
		framebuffer_info.attachmentCount = 2;
		attachments[1] = info->depth_texture.image_view;
	}

	for (u32 i = 0; i < info->draw_framebuffers.get_size(); i++) {	
		if (render_context.anti_aliasing) {
			attachments[2] = (*textures)[i].image_view;
		} else {
			attachments[0] = (*textures)[i].image_view;
		}

		if (vkCreateFramebuffer(info->device, &framebuffer_info, nullptr, &info->draw_framebuffers[i]) != VK_SUCCESS) {
			logprint("vulkan_create_frame_buffers()", "failed to create framebuffer\n");
			return 1;
		}
	}

	return 0;
}

internal bool8
vulkan_create_swap_chain_framebuffers(Vulkan_Info *info) {	
	info->swap_chain_framebuffers.resize(info->swap_chain_textures.get_size());

	VkFramebufferCreateInfo framebuffer_info = {};
	framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass      = info->present_render_pass;
	framebuffer_info.attachmentCount = 1;
	framebuffer_info.width           = info->swap_chain_extent.width;
	framebuffer_info.height          = info->swap_chain_extent.height;
	framebuffer_info.layers          = 1;
	
	for (u32 i = 0; i < info->swap_chain_framebuffers.get_size(); i++) {
		framebuffer_info.pAttachments = &info->swap_chain_textures[i].image_view;

		if (vkCreateFramebuffer(info->device, &framebuffer_info, nullptr, &info->swap_chain_framebuffers[i]) != VK_SUCCESS) {
			logprint("vulkan_create_frame_buffers()", "failed to create framebuffer\n");
			return 1;
		}
	}

	return 0;
	
}

internal void
vulkan_create_command_pool(Vulkan_Info *info) {	
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = info->indices.graphics_and_compute_family.index;

	if (vkCreateCommandPool(info->device, &pool_info, nullptr, &info->command_pool) != VK_SUCCESS) {
		logprint("vulkan_create_command_pool()", "failed to create command pool\n");
	}
}

internal void
vulkan_create_command_buffers(VkCommandBuffer *command_buffers, u32 num_of_buffers) {
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = vulkan_info.command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = num_of_buffers;

	if (vkAllocateCommandBuffers(vulkan_info.device, &alloc_info, command_buffers) != VK_SUCCESS) {
		logprint("vulkan_create_command_buffers()", "failed to allocate command buffers\n");
	}
}

internal void
vulkan_create_sync_objects(VkDevice device, Vulkan_Frame frames[MAX_FRAMES_IN_FLIGHT]) {
	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (u32 frame_index = 0; frame_index < MAX_FRAMES_IN_FLIGHT; frame_index++) {
		Vulkan_Frame *frame = &frames[frame_index];

		for (u32 semaphore_index = 0; semaphore_index < ARRAY_COUNT(frame->semaphores); semaphore_index++) {			
			if (vkCreateSemaphore(device, &semaphore_info, nullptr, &frame->semaphores[semaphore_index]) != VK_SUCCESS) {
	    	logprint("vulkan_create_sync_objects()", "failed to create semaphores\n");				
			}
		}

		for (u32 fence_index = 0; fence_index < ARRAY_COUNT(frame->fences); fence_index++) {			
			if (vkCreateFence(device, &fence_info, nullptr, &frame->fences[fence_index]) != VK_SUCCESS) {
	    	logprint("vulkan_create_sync_objects()", "failed to create fences\n");				
			}
		}
	}
}

internal u32
vulkan_find_memory_type(VkPhysicalDevice physical_device, u32 type_filter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
		if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	logprint("vulkan_find_memory_type()", "failed to find suitable memory type\n");
	return 0;
}

internal void
vulkan_create_buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory) {
	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
		logprint("vulkan_create_buffer()", "failed to create buffer\n");
		return;
	}

	VkMemoryRequirements memory_requirements = {};
	vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);
	
	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = memory_requirements.size;
	allocate_info.memoryTypeIndex = vulkan_find_memory_type(physical_device, memory_requirements.memoryTypeBits, properties);
	
	if (vkAllocateMemory(device, &allocate_info, nullptr, &buffer_memory) != VK_SUCCESS) {
		logprint("vulkan_create_buffer()", "failed to allocate buffer memory\n");
		return;
	}

	vkBindBufferMemory(device, buffer, buffer_memory, 0);
}

internal void
vulkan_create_buffer(Vulkan_Buffer *buffer, VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
	buffer->size = size;
	vulkan_create_buffer(device, physical_device, buffer->size, usage, properties, buffer->handle, buffer->memory);
	
	if (properties | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		vkMapMemory(device, buffer->memory, 0, VK_WHOLE_SIZE, 0, &buffer->data);
	}
}

internal VkCommandBuffer
vulkan_begin_single_time_commands(VkDevice device, VkCommandPool command_pool) {
	VkCommandBufferAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandPool = command_pool;
	allocate_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(device, &allocate_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(command_buffer, &begin_info);

	return command_buffer;
}

internal void
vulkan_end_single_time_commands(VkCommandBuffer command_buffer, VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue) {
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphics_queue);

	vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

internal void
vulkan_copy_buffer(Vulkan_Info *info, VkBuffer src_buffer, VkBuffer dest_buffer, VkDeviceSize size, u32 src_offset, u32 dest_offset) {
	VkCommandBuffer command_buffer = vulkan_begin_single_time_commands(info->device, info->command_pool);

	VkBufferCopy copy_region = {};
	copy_region.srcOffset = src_offset;  // Optional
	copy_region.dstOffset = dest_offset; // Optional
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, src_buffer, dest_buffer, 1, &copy_region);

	vulkan_end_single_time_commands(command_buffer, info->device, info->command_pool, info->graphics_queue);
}

// return the offset to the memory set in the buffer
internal void
vulkan_update_buffer(Vulkan_Info *info, VkBuffer *buffer, VkDeviceMemory *memory, void *in_data, u32 in_data_size, u32 offset) {
	VkDeviceSize buffer_size = in_data_size;
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	
	vulkan_create_buffer(info->device, 
						 info->physical_device,
						 buffer_size, 
						 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 staging_buffer,
						 staging_buffer_memory);

	void *data;
	vkMapMemory(info->device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, in_data, buffer_size);
	vkUnmapMemory(info->device, staging_buffer_memory);

	vulkan_copy_buffer(info, staging_buffer, *buffer, buffer_size, 0, offset);
	
	vkDestroyBuffer(info->device, staging_buffer, nullptr);
	vkFreeMemory(info->device, staging_buffer_memory, nullptr);
}

internal void*
vulkan_get_memory(VkDeviceMemory memory, u32 size) {
	void *return_data = platform_malloc(size);
	
	void *data;
	vkMapMemory(vulkan_info.device, memory, 0, size, 0, &data);
	memcpy(return_data, data, size);
	vkUnmapMemory(vulkan_info.device, memory);

	return return_data;
}

//
// Helper Functions
//

// returns next size that will align after in size
internal VkDeviceSize
vulkan_get_alignment(VkDeviceSize in, u32 alignment) {
	while(in % alignment != 0) {
		in++;
	}
	return in;
}

// returns true if it looped back to the
internal u32
vulkan_get_next_offset(u32 *offset, u32 in_data_size, u32 max_offset, bool8 allow_looping) {
	u32 return_offset = *offset;
  *offset += in_data_size;

	if (*offset < max_offset)
		return return_offset;

	if (allow_looping) {
#if DEBUG
		logprint("vulkan_get_next_offset()", "Looping offset back to beginning\n");
#endif
		*offset = 0;
		return *offset;
	} else {
		logprint("vulkan_get_next_offset()", "Went passed end of buffer with looping not allowed\n");
		ASSERT(0);

		return *offset;
	}
}

internal u32
vulkan_convert_shader_stage(u32 shader_stage) {
	switch(shader_stage) {
		case SHADER_STAGE_VERTEX:                  return VK_SHADER_STAGE_VERTEX_BIT;
		case SHADER_STAGE_TESSELLATION_CONTROL:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case SHADER_STAGE_TESSELLATION_EVALUATION: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case SHADER_STAGE_GEOMETRY:                return VK_SHADER_STAGE_GEOMETRY_BIT;
		case SHADER_STAGE_FRAGMENT:                return VK_SHADER_STAGE_FRAGMENT_BIT;
		case SHADER_STAGE_COMPUTE:                 return VK_SHADER_STAGE_COMPUTE_BIT;
		default: {
			logprint("vulkan_convert_shader_stage()", "%d is not a render shader stage\n", shader_stage);
			return 0;
		}
	};
}

internal VkDescriptorType
vulkan_convert_descriptor_type(u32 descriptor_type) {
	u32 vulkan_type = 0;

	switch(descriptor_type) {
		case DESCRIPTOR_TYPE_UNIFORM_BUFFER:         vulkan_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;         break;
		case DESCRIPTOR_TYPE_SAMPLER:                vulkan_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; break;
		case DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: vulkan_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; break;
		case DESCRIPTOR_TYPE_STORAGE_BUFFER:         vulkan_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;         break;
		default: {
			logprint("vulkan_convert_descriptor_type()", "%d is not a render descriptor type\n", descriptor_type);
		} break;
	}

	return (VkDescriptorType)vulkan_type;
}

//
// Pipeline/Shaders
//

// Attempt at shader objects

internal VkResult 
vulkan_create_shaders_ext(VkDevice device, u32 createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders) {
    PFN_vkCreateShadersEXT func = (PFN_vkCreateShadersEXT)vkGetInstanceProcAddr(vulkan_info.instance, "vkCreateShadersEXT");
	PFN_vkCreateShadersEXT test = (PFN_vkCreateShadersEXT)vkGetDeviceProcAddr(device, "vkCreateShadersEXT");
    if (func != nullptr) {
		VkShaderEXT huh;
        return func(device, 1, pCreateInfos, nullptr, &huh);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

internal VkShaderEXT
vulkan_create_shader_object(VkDevice device, File code, u32 stage, VkDescriptorSetLayout *layout) {
	VkShaderCreateInfoEXT create_info = {};
	create_info.sType     = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
	create_info.pNext     = NULL;
	create_info.flags     = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
	create_info.stage     = VK_SHADER_STAGE_VERTEX_BIT;
	create_info.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
	create_info.codeType  = VK_SHADER_CODE_TYPE_SPIRV_EXT;
	create_info.codeSize  = code.size;
	create_info.pCode     = code.memory;
	create_info.pName     = "main"; 
	create_info.setLayoutCount         = 1;
	create_info.pSetLayouts            = layout;
	create_info.pushConstantRangeCount = 0;     
	create_info.pPushConstantRanges    = nullptr;
	create_info.pSpecializationInfo = NULL;   

	VkShaderEXT shader[2] = {};
	if (vulkan_create_shaders_ext(device, 1, &create_info, nullptr, shader) != VK_SUCCESS) {
		logprint("vulkan_create_shader_object()", "failed to create shader object\n");
	}	
	return shader[0];
}

internal VkShaderModule
vulkan_create_shader_module(VkDevice device, File code) {
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size;
	create_info.pCode = (u32*)code.memory;

	VkShaderModule shader_module;
	if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
		logprint("vulkan_create_shader_module()", "failed to create shader module\n");
	}

	return shader_module;
}

inline VkBool32
vulkan_bool(bool8 in) {
	if (in) return VK_TRUE;
	else    return VK_FALSE;
}

bool8 vulkan_create_graphics_pipeline(Shader *shader, VkRenderPass render_pass) {
	GFX_Pipeline *pipeline = &shader->pipeline;

	u32 shader_stages_index = 0;
	VkPipelineShaderStageCreateInfo shader_stages[SHADER_STAGES_AMOUNT] = {};
	VkShaderModule shader_modules[SHADER_STAGES_AMOUNT] = {};
	
	// Creating shader modules
	for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
		if (shader->spirv_files[i].size == 0)
			continue;
		
		shader_modules[i] = vulkan_create_shader_module(vulkan_info.device, shader->spirv_files[i]);

		VkPipelineShaderStageCreateInfo shader_stage_info = {};
		shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_info.stage = (VkShaderStageFlagBits)vulkan_convert_shader_stage(i);
		shader_stage_info.module = shader_modules[i];
		shader_stage_info.pName = "main";
		shader_stages[shader_stages_index++] = shader_stage_info;
	}

	// Copy into arrays

	VkDescriptorSetLayout descriptor_set_layouts[5];
	for (u32 i = 0; i < shader->set.layouts_count; i++) {
		descriptor_set_layouts[i] = shader->set.layouts[i]->descriptor_set_layout;
	}

	VkPushConstantRange push_constant_ranges[5] = {};
	for (u32 i = 0; i < shader->set.push_constants_count; i++) {
		VkPushConstantRange range = {};
		range.stageFlags = vulkan_convert_shader_stage(shader->set.push_constants[i].shader_stage);
		range.offset = 0;
		range.size = shader->set.push_constants[i].size;
		push_constant_ranges[i] = range;
	}

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount         = shader->set.layouts_count; 
	pipeline_layout_info.pSetLayouts            = descriptor_set_layouts;      
	pipeline_layout_info.pushConstantRangeCount = shader->set.push_constants_count; 
	pipeline_layout_info.pPushConstantRanges    = push_constant_ranges;          

	if (vkCreatePipelineLayout(vulkan_info.device, &pipeline_layout_info, nullptr, &pipeline->layout) != VK_SUCCESS) {
		logprint("vulkan_create_graphics_pipeline()", "failed to create pipeline layout\n");
	}

	VkDynamicState dynamic_states[] = { 
		VK_DYNAMIC_STATE_VIEWPORT, 
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE
	};
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = ARRAY_COUNT(dynamic_states);
	dynamic_state.pDynamicStates = dynamic_states;

	// Create the pipeline vertex input info
	u32 vertex_size = 0;
	for (u32 i = 0; i < shader->vertex_info.attributes_count; i++) {
		switch(shader->vertex_info.formats[i]) {
			case VECTOR2: vertex_size += sizeof(Vector2); break;
			case VECTOR3: vertex_size += sizeof(Vector3); break;
		}
	}

	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = vertex_size;
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	
	VkVertexInputAttributeDescription attribute_descriptions[5] = {}; // 5 = vertex max attributes
	for (u32 i = 0; i < shader->vertex_info.attributes_count; i++) {
		attribute_descriptions[i].binding = 0;
		attribute_descriptions[i].location = i;
		attribute_descriptions[i].format = convert_to_vulkan(shader->vertex_info.formats[i]);
		attribute_descriptions[i].offset = shader->vertex_info.offsets[i];
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount   = 1;
	vertex_input_info.pVertexBindingDescriptions      = &binding_description;         // Optional
	vertex_input_info.vertexAttributeDescriptionCount = shader->vertex_info.attributes_count;
	vertex_input_info.pVertexAttributeDescriptions    = attribute_descriptions;       // Optional

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount  = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable        = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth               = 1.0f;
	rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable         = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp          = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor    = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable   = VK_TRUE;
	multisampling.rasterizationSamples  = vulkan_info.msaa_samples;
	multisampling.minSampleShading      = .2f;      // Optional
	multisampling.pSampleMask           = nullptr;  // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable      = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable         = VK_TRUE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable     = VK_FALSE;
	color_blending.logicOp           = VK_LOGIC_OP_COPY; // Optional
	color_blending.attachmentCount   = 1;
	color_blending.pAttachments      = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;             // Optional
	color_blending.blendConstants[1] = 0.0f;             // Optional
	color_blending.blendConstants[2] = 0.0f;             // Optional
	color_blending.blendConstants[3] = 0.0f;             // Optional
	
	VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
	depth_stencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable       = vulkan_bool(pipeline->depth_test);
	depth_stencil.depthWriteEnable      = VK_TRUE;
	depth_stencil.depthCompareOp        = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.minDepthBounds        = 0.0f;               // Optional
	depth_stencil.maxDepthBounds        = 1.0f;               // Optional
	depth_stencil.stencilTestEnable     = VK_FALSE;
	depth_stencil.front                 = {};                 // Optional
	depth_stencil.back                  = {};                 // Optional
	
	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.stageCount          = 2;
	pipeline_create_info.pStages             = shader_stages;
	pipeline_create_info.pVertexInputState   = &vertex_input_info;
	pipeline_create_info.pInputAssemblyState = &input_assembly;
	pipeline_create_info.pViewportState      = &viewport_state;
	pipeline_create_info.pRasterizationState = &rasterizer;
	pipeline_create_info.pMultisampleState   = &multisampling;
	pipeline_create_info.pDepthStencilState  = &depth_stencil;         // Optional
	pipeline_create_info.pColorBlendState    = &color_blending;
	pipeline_create_info.pDynamicState       = &dynamic_state;
	pipeline_create_info.layout              = pipeline->layout;
	pipeline_create_info.renderPass          = render_pass;
	pipeline_create_info.subpass             = 0;
	pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE;        // Optional
	pipeline_create_info.basePipelineIndex   = -1;                    // Optional

	if (vkCreateGraphicsPipelines(vulkan_info.device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline->handle) != VK_SUCCESS) {
		logprint("vulkan_create_graphics_pipeline()", "failed to create graphics pipelines\n");
	}

	for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
		if (shader_modules[i] != 0)
			vkDestroyShaderModule(vulkan_info.device, shader_modules[i], nullptr);
	}

	shader->pipeline.compiled = TRUE;

	return false;
}
void vulkan_create_graphics_pipeline(Shader *shader) {
	vulkan_create_graphics_pipeline(shader, vulkan_info.draw_render_pass);
}

void vulkan_create_compute_pipeline(Shader *shader) {
	GFX_Pipeline *pipeline = &shader->pipeline;

	u32 shader_stages_index = 0;
	VkPipelineShaderStageCreateInfo shader_stage = {};
	VkShaderModule shader_module = {};
	
	// Creating shader modules
	shader_module = vulkan_create_shader_module(vulkan_info.device, shader->spirv_files[SHADER_STAGE_COMPUTE]);

	VkPipelineShaderStageCreateInfo shader_stage_info = {};
	shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_info.stage =  VK_SHADER_STAGE_COMPUTE_BIT;
	shader_stage_info.module = shader_module;
	shader_stage_info.pName = "main";
	shader_stage = shader_stage_info;

	// Copy into arrays

	VkDescriptorSetLayout descriptor_set_layouts[5];
	for (u32 i = 0; i < shader->set.layouts_count; i++) {
		descriptor_set_layouts[i] = shader->set.layouts[i]->descriptor_set_layout;
	}

	VkPushConstantRange push_constant_ranges[5] = {};
	for (u32 i = 0; i < shader->set.push_constants_count; i++) {
		VkPushConstantRange range = {};
		range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		range.offset = 0;
		range.size = shader->set.push_constants[i].size;
		push_constant_ranges[i] = range;
	}

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount         = shader->set.layouts_count; 
	pipeline_layout_info.pSetLayouts            = descriptor_set_layouts;      
	pipeline_layout_info.pushConstantRangeCount = shader->set.push_constants_count; 
	pipeline_layout_info.pPushConstantRanges    = push_constant_ranges;          

	if (vkCreatePipelineLayout(vulkan_info.device, &pipeline_layout_info, nullptr, &pipeline->layout) != VK_SUCCESS) {
		logprint("vulkan_create_graphics_pipeline()", "failed to create pipeline layout\n");
	}

	VkComputePipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_create_info.layout = pipeline->layout;
	pipeline_create_info.stage = shader_stage;

	if (vkCreateComputePipelines(vulkan_info.device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline->handle) != VK_SUCCESS) {
		logprint("vulkan_create_compute_pipeline()", "failed to create compute pipelines\n");
	}

	vkDestroyShaderModule(vulkan_info.device, shader_module, nullptr);

	shader->pipeline.compiled = TRUE;
}

//
// Images
//

internal void
vulkan_generate_mipmaps(VkImage image, VkFormat image_format, u32 tex_width, u32 tex_height, u32 mip_levels) {
	// Check if image format supports linear blitting
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(vulkan_info.physical_device, image_format, &format_properties);

	if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
	    logprint("vulkan_generate_mipmaps()", "texture image format does not support linear blitting!");
	}

	VkCommandBuffer command_buffer = vulkan_begin_single_time_commands(vulkan_info.device, vulkan_info.command_pool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	s32 mip_width = tex_width;
	s32 mip_height = tex_height;
	for (u32 i = 1; i < mip_levels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mip_width, mip_height, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mip_width > 1)  mip_width  /= 2;
  	if (mip_height > 1) mip_height /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	vulkan_end_single_time_commands(command_buffer, vulkan_info.device, vulkan_info.command_pool, vulkan_info.graphics_queue);
}



internal void
vulkan_create_image(VkDevice device, VkPhysicalDevice physical_device, u32 width, u32 height, u32 mip_levels, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &image_memory) {
	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = 1;
  image_info.mipLevels = mip_levels;
  image_info.arrayLayers = 1;
  image_info.format = format;
  image_info.tiling = tiling;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = usage;
  image_info.samples = num_samples;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &image_info, nullptr, &image) != VK_SUCCESS) {
		logprint("vulkan_create_image()", "failed to create image\n");
	}

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device, image, &memory_requirements);

	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = memory_requirements.size;
	allocate_info.memoryTypeIndex = vulkan_find_memory_type(physical_device, memory_requirements.memoryTypeBits, properties);
	
	if (vkAllocateMemory(device, &allocate_info, nullptr, &image_memory)) {
		logprint("vulkan_create_image()", "failed to allocate image memory\n");
	}

	vkBindImageMemory(device, image, image_memory, 0);
}

internal bool8
vulkan_has_stencil_component(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

internal void
vulkan_transition_image_layout(VkCommandBuffer command_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, u32 mip_levels) {
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mip_levels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO

	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags destination_stage;

	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
	    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	    if (vulkan_has_stencil_component(format)) {
	        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	    }
	} else {
	    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
	    barrier.srcAccessMask = 0;
	    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if ((old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ||
						 (old_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR      && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)     ||
						 (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)         ) {
	    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
	    barrier.srcAccessMask = 0;
	    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	    destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
	    logprint("vulkan_create_texture_image()", "unsupported layout transition\n");
	}

	vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

internal void
vulkan_copy_buffer_to_image(Vulkan_Info *info, VkBuffer buffer, VkImage image, u32 width, u32 height) {
	VkCommandBuffer command_buffer = vulkan_begin_single_time_commands(info->device, info->command_pool);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	
	vulkan_end_single_time_commands(command_buffer, info->device, info->command_pool, info->graphics_queue);
}

internal VkImageView
vulkan_create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, u32 mip_levels) {
	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect_flags;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = mip_levels;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	VkImageView image_view;
    if (vkCreateImageView(device, &view_info, nullptr, &image_view) != VK_SUCCESS) {
        logprint("vulkan_create_image_view()", "failed to create texture image view\n");
    }

    return image_view;
}

internal void
vulkan_create_depth_resources(Vulkan_Info *info) {
	info->depth_texture.image_format = vulkan_find_depth_format(info->physical_device);
	vulkan_create_image(info->device, info->physical_device, render_context.resolution.width, render_context.resolution.height, 1, info->msaa_samples, info->depth_texture.image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, info->depth_texture.image, info->depth_texture.image_memory);
	info->depth_texture.image_view = vulkan_create_image_view(info->device, info->depth_texture.image, info->depth_texture.image_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	
	VkCommandBuffer command_buffer = vulkan_begin_single_time_commands(info->device, info->command_pool);
	vulkan_transition_image_layout(command_buffer, info->depth_texture.image, info->depth_texture.image_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);	
	vulkan_end_single_time_commands(command_buffer, info->device, info->command_pool, info->graphics_queue);
}

internal void
vulkan_create_color_resources() {
	vulkan_info.color_texture.image_format = vulkan_info.draw_textures[0].image_format;

  vulkan_create_image(vulkan_info.device, vulkan_info.physical_device, render_context.resolution.width, render_context.resolution.height, 1, vulkan_info.msaa_samples, vulkan_info.color_texture.image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan_info.color_texture.image, vulkan_info.color_texture.image_memory);
  vulkan_info.color_texture.image_view = vulkan_create_image_view(vulkan_info.device, vulkan_info.color_texture.image, vulkan_info.color_texture.image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

// TODO: Make it so that images can be created asynchronously (vulkan-tutorial = Texture mapping/Images)

//
// Texture images
//



internal Vulkan_Texture*
vulkan_create_texture_image(Bitmap *bitmap) {
	Vulkan_Info *info = &vulkan_info;

	Vulkan_Texture *texture = (Vulkan_Texture *)platform_malloc(sizeof(Vulkan_Texture));
	*texture = {};

	if (bitmap->channels == 1) {
		texture->image_format = VK_FORMAT_R8_UNORM;
	}

  VkDeviceSize image_size = bitmap->width * bitmap->height * bitmap->channels;

  VkBuffer staging_buffer;
  VkDeviceMemory staging_buffer_memory;

  vulkan_create_buffer(info->device, info->physical_device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

	void *data;
	vkMapMemory(info->device, staging_buffer_memory, 0, image_size, 0, &data);
	memcpy(data, bitmap->memory, image_size);
	vkUnmapMemory(info->device, staging_buffer_memory);

	vulkan_create_image(info->device, info->physical_device, bitmap->width, bitmap->height, bitmap->mip_levels, VK_SAMPLE_COUNT_1_BIT, texture->image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->image, texture->image_memory);

	VkCommandBuffer command_buffer = vulkan_begin_single_time_commands(info->device, info->command_pool);
	vulkan_transition_image_layout(command_buffer, texture->image, texture->image_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, bitmap->mip_levels);
	vulkan_end_single_time_commands(command_buffer, info->device, info->command_pool, info->graphics_queue);
	
	vulkan_copy_buffer_to_image(info, staging_buffer, texture->image, (u32)bitmap->width, (u32)bitmap->height);
	//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
	//vulkan_transition_image_layout(info, texture->image, texture->image_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, bitmap->mip_levels);

	vkDestroyBuffer(info->device, staging_buffer, nullptr);
	vkFreeMemory(info->device, staging_buffer_memory, nullptr);

	vulkan_generate_mipmaps(texture->image, texture->image_format, bitmap->width, bitmap->height, bitmap->mip_levels);

	return texture;
}

internal void
vulkan_create_sampler(VkSampler *sampler, u32 texture_parameters, u32 mip_levels) {
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(vulkan_info.physical_device, &properties);
	
	VkSamplerCreateInfo sampler_info = {};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	switch(texture_parameters) {
		case TEXTURE_PARAMETERS_DEFAULT:
		sampler_info.minFilter    = VK_FILTER_NEAREST;
		sampler_info.magFilter    = VK_FILTER_NEAREST;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;

		case TEXTURE_PARAMETERS_CHAR:
		sampler_info.minFilter    = VK_FILTER_LINEAR;
		sampler_info.magFilter    = VK_FILTER_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		break;
	}

	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = (float32)mip_levels;

	if (vkCreateSampler(vulkan_info.device, &sampler_info, nullptr, sampler) != VK_SUCCESS) {
		logprint("vulkan_create_sampler()", "failed to create sampler\n");
	}
}

void* vulkan_create_texture(Bitmap *bitmap, u32 texture_parameters) {
	if (bitmap->mip_levels == 0) {
		bitmap->mip_levels = (u32)floor(log2f((float32)max(bitmap->width, bitmap->height))) + 1;
	}   

	if (bitmap->channels == 0) {
		logprint("vulkan_create_texture()", "bitmap not loaded\n");
		return 0;
	}

	Vulkan_Texture *texture = vulkan_create_texture_image(bitmap);
  texture->image_view = vulkan_create_image_view(vulkan_info.device, texture->image, texture->image_format, VK_IMAGE_ASPECT_COLOR_BIT, bitmap->mip_levels);
  vulkan_create_sampler(&texture->sampler, texture_parameters, bitmap->mip_levels);

	bitmap->gpu_info = (void*)texture;

  return (void*)texture;
}

void vulkan_destroy_texture(void *gpu_handle) {
	Vulkan_Texture *texture = (Vulkan_Texture *)gpu_handle;
	if (texture == 0)
		return;
	
	vkDestroySampler(vulkan_info.device, texture->sampler, nullptr);
	vkDestroyImageView(vulkan_info.device, texture->image_view, nullptr);
	vkDestroyImage(vulkan_info.device, texture->image, nullptr);
	vkFreeMemory(vulkan_info.device, texture->image_memory, nullptr);
}

void vulkan_delete_texture(Bitmap *bitmap) {
	if (bitmap->width == 0)
		return;

	vulkan_destroy_texture(bitmap->gpu_info);
}


//
// Init, cleanup and recreation
//

internal void
vulkan_create_render_image_views(Vulkan_Info *info) {
	info->draw_textures.resize(info->swap_chain_textures.get_size());

	for (u32 i = 0; i < info->swap_chain_textures.get_size(); i++) {
		info->swap_chain_textures[i].image_view = vulkan_create_image_view(info->device, info->swap_chain_textures[i].image, info->swap_chain_textures[i].image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		info->draw_textures[i].image_format = info->swap_chain_textures[i].image_format;
		//info->draw_textures[i].image_format = VK_FORMAT_B8G8R8A8_UNORM;
		vulkan_create_image(info->device, info->physical_device, render_context.resolution.width, render_context.resolution.height, 1, VK_SAMPLE_COUNT_1_BIT, info->draw_textures[i].image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, info->draw_textures[i].image, info->draw_textures[i].image_memory);
		info->draw_textures[i].image_view = vulkan_create_image_view(info->device, info->draw_textures[i].image, info->draw_textures[i].image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		vulkan_create_sampler(&info->draw_textures[i].sampler, TEXTURE_PARAMETERS_DEFAULT, 1);
	}
}

internal void
vulkan_cleanup_swap_chain(Vulkan_Info *info) {
	for (u32 i = 0; i < info->swap_chain_framebuffers.get_size(); i++) {
		vkDestroyFramebuffer(info->device, info->swap_chain_framebuffers[i], nullptr);
	}

	for (u32 i = 0; i < info->draw_framebuffers.get_size(); i++) {
		vkDestroyFramebuffer(info->device, info->draw_framebuffers[i], nullptr);
	}

	for (u32 i = 0; i < info->swap_chain_textures.get_size(); i++) {
		vkDestroyImageView(info->device, info->swap_chain_textures[i].image_view, nullptr);
		vulkan_destroy_texture(&info->draw_textures[i]);
	}

	vkDestroySwapchainKHR(info->device, info->swap_chains[0], nullptr);
}

void vulkan_pipeline_cleanup(Render_Pipeline *pipe) {
	vkDestroyPipeline(vulkan_info.device, pipe->handle, nullptr);
	vkDestroyPipelineLayout(vulkan_info.device, pipe->layout, nullptr);
}

internal void
vulkan_recreate_pipelines(Assets *assets) {
	for (u32 i = 0; i < assets->types[ASSET_TYPE_SHADER].num_of_assets; i++) {
		Shader *shader = &assets->types[ASSET_TYPE_SHADER].data[i].shader;
		GFX_Pipeline *pipeline = &shader->pipeline;

		if (!pipeline->compiled) {
			logprint("vulkan_recreate_pipelines()", "pipeline not compiled (%d)\n", i);
			continue;
		}
		
		vulkan_pipeline_cleanup(pipeline);
		
		if (!pipeline->compute) {
			render_create_graphics_pipeline(shader);
		} else {
	    render_create_compute_pipeline(shader);
		}
	}
}

internal void
vulkan_recreate_swap_chain(Vulkan_Info *info, Vector2_s32 window_dim, Assets *assets) {
	vkDeviceWaitIdle(info->device);

	vulkan_cleanup_swap_chain(info);
	
	vulkan_destroy_texture(&info->color_texture);
	vulkan_destroy_texture(&info->depth_texture);

	vkDestroyRenderPass(info->device, info->draw_render_pass, nullptr);
	vkDestroyRenderPass(info->device, info->present_render_pass, nullptr);
	
	if (render_context.anti_aliasing)
		info->msaa_samples = vulkan_get_max_usable_sample_count();
	else
		info->msaa_samples = VK_SAMPLE_COUNT_1_BIT;
		
	vulkan_create_draw_render_pass(info);
	vulkan_create_present_render_pass();
	
	vulkan_create_swap_chain(info, window_dim);
	vulkan_create_render_image_views(info);
	vulkan_create_color_resources();
	vulkan_create_depth_resources(info);
	vulkan_create_draw_framebuffers(info);
	vulkan_create_swap_chain_framebuffers(info);
	
	vulkan_info.draw_render_pass_info.renderPass = vulkan_info.draw_render_pass;
	vulkan_info.present_render_pass_info.renderPass = vulkan_info.present_render_pass;

	vulkan_recreate_pipelines(assets);	
}


void vulkan_assets_cleanup(Assets *assets) {
	for (u32 i = 0; i < assets->num_of_assets; i++) {
		Asset *asset = &assets->data[i];
		switch(asset->type) {
		    case ASSET_TYPE_FONT: {
					Font *font = (Font *)&asset->font;
					for (s32 j = 0; j < ARRAY_COUNT(font->cache->bitmaps); j++) {
		    		vulkan_delete_texture(&font->cache->bitmaps[j].bitmap);
		  		}
				} break;

		    case ASSET_TYPE_BITMAP: {
					Bitmap *bitmap = (Bitmap *)&asset->bitmap;
		  		vulkan_delete_texture(bitmap);
				} break;

		    case ASSET_TYPE_MODEL: {
					Model *model = (Model *)&asset->model;

		      for (u32 j = 0; j < model->meshes_count; j++) {
						vulkan_delete_texture(&model->meshes[j].material.diffuse_map);
		      }
				} break;
				
		    case ASSET_TYPE_SHADER: {
						Shader *shader = (Shader *)&asset->shader;
		    		vulkan_pipeline_cleanup(&shader->pipeline);
		    } break;
		    case ASSET_TYPE_AUDIO: break;
		}
	}
}

void vulkan_wait_frame() {
	vkDeviceWaitIdle(vulkan_info.device);
}

internal void
vulkan_cleanup_buffer(Vulkan_Buffer buffer) {
	vkDestroyBuffer(vulkan_info.device, buffer.handle, nullptr);
	vkFreeMemory(vulkan_info.device, buffer.memory, nullptr);
}

void vulkan_cleanup_frame(VkDevice device, Vulkan_Frame *frame) {
		for (u32 semaphore_index = 0; semaphore_index < ARRAY_COUNT(frame->semaphores); semaphore_index++) {			
			vkDestroySemaphore(device, frame->semaphores[semaphore_index], nullptr);
		}

		for (u32 fence_index = 0; fence_index < ARRAY_COUNT(frame->fences); fence_index++) {			
	    vkDestroyFence(device, frame->fences[fence_index], nullptr);
		}
}

void vulkan_cleanup_layouts(Layout *layouts, u32 layouts_count) {
	for (u32 i = 0; i < layouts_count; i++) {
		if (layouts[i].descriptor_set_layout != 0)
			vkDestroyDescriptorSetLayout(vulkan_info.device, layouts[i].descriptor_set_layout, nullptr);
	}
}

void vulkan_cleanup() {
	Vulkan_Info *info = &vulkan_info;
	vulkan_cleanup_swap_chain(info);

	vulkan_destroy_texture(&info->depth_texture);
	vulkan_destroy_texture(&info->color_texture);

	vkDestroyDescriptorPool(vulkan_info.device, vulkan_info.descriptor_pool, nullptr);

	for (u32 i = 0; i < ARRAY_COUNT(info->buffers); i++) {
		vulkan_cleanup_buffer(info->buffers[i]);
	}
		
	vulkan_pipeline_cleanup(&present_pipeline);
	
	vkDestroyRenderPass(info->device, info->draw_render_pass, nullptr);
	vkDestroyRenderPass(info->device, info->present_render_pass, nullptr);

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vulkan_cleanup_frame(info->device, &info->frames[i]);
	}
	
	vkDestroyCommandPool(info->device, info->command_pool, nullptr);
	vkDestroyDevice(info->device, nullptr);
	vkDestroySurfaceKHR(info->instance, info->surface, nullptr);
	
	if (info->validation_layers.enable)
		vulkan_destroy_debug_utils_messenger_ext(info->instance, info->debug_messenger, nullptr);

	vkDestroyInstance(info->instance, nullptr);
}

internal VkExtent2D
vulkan_get_extent() {	
	VkExtent2D extent;
	
	if (render_context.resolution_scaling) {
		VkExtent2D resolution = {};
		resolution.width = render_context.resolution.width;
		resolution.height = render_context.resolution.height;
		extent = resolution;
	} else {
		extent = vulkan_info.swap_chain_extent;
	}

	return extent;
}

internal void
vulkan_init_presentation_settings(Vulkan_Info *info) {
	Vulkan_Frame *frame = &vulkan_info.frames[gfx.current_frame];

	// Start frame
	info->begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info->begin_info.flags = 0;				   // Optional
	info->begin_info.pInheritanceInfo = nullptr; // Optional
/*
	info->clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
*/
	info->clear_values[1].depthStencil = {1.0f, 0};

	info->draw_render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info->draw_render_pass_info.renderPass = info->draw_render_pass;
	info->draw_render_pass_info.renderArea.offset = {0, 0};
	info->draw_render_pass_info.framebuffer = info->draw_framebuffers[info->image_index];
	info->draw_render_pass_info.renderArea.extent = vulkan_get_extent();
	info->draw_render_pass_info.clearValueCount = ARRAY_COUNT(info->clear_values);
	info->draw_render_pass_info.pClearValues = info->clear_values;
	
	info->present_render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info->present_render_pass_info.renderPass = info->present_render_pass;
	info->present_render_pass_info.framebuffer = info->swap_chain_framebuffers[info->image_index];
	info->present_render_pass_info.renderArea.offset = {0, 0};
	info->present_render_pass_info.renderArea.extent = info->swap_chain_extent;
	info->present_render_pass_info.clearValueCount = ARRAY_COUNT(info->clear_values);
	info->present_render_pass_info.pClearValues = info->clear_values;

	// End of frame
	info->submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info->submit_info.waitSemaphoreCount = 1;
	info->submit_info.pWaitSemaphores = &frame->image_available_semaphore;
	info->submit_info.pWaitDstStageMask = info->wait_stages;
	info->submit_info.commandBufferCount = 1;
	info->submit_info.pCommandBuffers = &frame->command_buffer;	
	info->submit_info.signalSemaphoreCount = 1;
	info->submit_info.pSignalSemaphores = &frame->render_finished_semaphore;

	info->present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info->present_info.waitSemaphoreCount = 1;
	info->present_info.pWaitSemaphores = &frame->render_finished_semaphore;
	info->present_info.swapchainCount = ARRAY_COUNT(info->swap_chains);
	info->present_info.pSwapchains = info->swap_chains;
	info->present_info.pImageIndices = &info->image_index;
	info->present_info.pResults = nullptr; // Optional
}

bool8 vulkan_sdl_init(SDL_Window *sdl_window) {
	Vulkan_Info *info = &vulkan_info;

	if (info->validation_layers.enable && !vulkan_check_validation_layer_support(info->validation_layers)) {
		logprint("vulkan_init()", "validation layers requested, but not avaiable\n");
	}

	u32 instance_extensions_count;
	if (SDL_Vulkan_GetInstanceExtensions(sdl_window, &instance_extensions_count, NULL) == SDL_FALSE) {
		logprint("main", "nullptr SDL_Vulkan_GetInstanceExtensions failed\n");
		return 1;
	}

	const char **instance_extensions = ARRAY_MALLOC(const char *, (instance_extensions_count + ARRAY_COUNT(info->extra_instance_extensions)));
	if (SDL_Vulkan_GetInstanceExtensions(sdl_window, &instance_extensions_count, instance_extensions) == SDL_FALSE) {
		logprint("main", "failed to get instance extensions\n");
		return 1;
	}
	instance_extensions[instance_extensions_count] = info->extra_instance_extensions[0];

	if (vulkan_create_instance(info, instance_extensions, instance_extensions_count + ARRAY_COUNT(info->extra_instance_extensions))) return 1;
	platform_free(instance_extensions);

	vulkan_setup_debug_messenger(info);

	if (SDL_Vulkan_CreateSurface(sdl_window, info->instance, &info->surface) == SDL_FALSE) {
		logprint("main", "vulkan surface failed being created\n");
		return 1;
	}

	// vulkan stuff
	if (vulkan_pick_physical_device(info)) return 1;
	
	info->uniform_buffer_min_alignment = info->physical_device_properties.limits.minUniformBufferOffsetAlignment;
	if (render_context.anti_aliasing)
		info->msaa_samples = vulkan_get_max_usable_sample_count();
	else
		info->msaa_samples = VK_SAMPLE_COUNT_1_BIT;
	
	vulkan_create_logical_device(info);
	
	vulkan_create_swap_chain(info, render_context.window_dim);
	vulkan_create_render_image_views(info);
	if (vulkan_create_draw_render_pass(info)) return 1;
	if (vulkan_create_present_render_pass()) return 1;
	
	vulkan_create_sync_objects(info->device, info->frames);
	vulkan_create_command_pool(info);

	vulkan_create_command_buffers(info->frames[0].command_buffers, 2);
	vulkan_create_command_buffers(info->frames[1].command_buffers, 2);

	vulkan_create_depth_resources(info);
	vulkan_create_color_resources();
	if (vulkan_create_draw_framebuffers(info)) return 1;
	if (vulkan_create_swap_chain_framebuffers(info)) return 1;

	//
	// Buffers
	//

	vulkan_create_buffer(&info->static_buffer, info->device, info->physical_device, VULKAN_STATIC_BUFFER_SIZE, 
											 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
											 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	
	vulkan_create_buffer(&info->dynamic_buffer, info->device, info->physical_device, VULKAN_STATIC_BUFFER_SIZE, 
											 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
											 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
	vulkan_create_buffer(&info->static_uniform_buffer, info->device, info->physical_device, VULKAN_STATIC_UNIFORM_BUFFER_SIZE, 
											 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
											 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
	vulkan_create_buffer(&info->dynamic_uniform_buffer, info->device, info->physical_device, VULKAN_DYNAMIC_UNIFORM_BUFFER_SIZE, 
											 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
											 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vulkan_create_buffer(&info->storage_buffer, info->device, info->physical_device, 500, 
											 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
											 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vulkan_create_buffer(&info->triangle_buffer, info->device, info->physical_device, 10000, 
											 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
											 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vulkan_init_presentation_settings(info);

	//
	// Create Descriptor Pool
	//

	VkDescriptorPoolSize pool_sizes[3] = {};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = 1000;
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = 25000;
	pool_sizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pool_sizes[2].descriptorCount = MAX_FRAMES_IN_FLIGHT * 128;

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = ARRAY_COUNT(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	pool_info.maxSets = 26000;

  VkResult result = vkCreateDescriptorPool(vulkan_info.device, &pool_info, nullptr, &vulkan_info.descriptor_pool);
  if (result != VK_SUCCESS) {
		logprint("vulkan_create_descriptor_pool()", "failed to create descriptor pool\n");
	}

	return 0;
}

//
// Render
//

void vulkan_dispatch(u32 group_count_x, u32 group_count_y, u32 group_count_z) {
	vkCmdDispatch(VK_CMD(vulkan_info), group_count_x, group_count_y, group_count_z);
}

void vulkan_clear_color(Vector4 color) {
	vulkan_info.clear_values[0].color = {{color.r, color.g, color.b, color.a}};
}

void vulkan_set_viewport(u32 window_width, u32 window_height) {
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float32)window_width;
	viewport.height = (float32)window_height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(VK_CMD(vulkan_info), 0, 1, &viewport);
}

void vulkan_set_scissor(s32 x, s32 y, u32 width, u32 height) {
	VkRect2D scissor;
	scissor.offset = { x, y };
	scissor.extent = { width, height };

	vkCmdSetScissor(VK_CMD(vulkan_info), 0, 1, &scissor);
}

void vulkan_bind_shader(const char *tag) {
	u32 id = find_asset_id(global_assets, tag);
	gfx.active_shader_id = id;

	Shader *shader = &global_assets->data[id].shader;
	vulkan_info.pipeline_layout = shader->pipeline.layout; // to use when binding sets later
	if (!shader->pipeline.compute) {
		vkCmdBindPipeline(VK_CMD(vulkan_info), VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.handle);
	} else {
		vkCmdBindPipeline(VK_CMD(vulkan_info), VK_PIPELINE_BIND_POINT_COMPUTE, shader->pipeline.handle);
	}
}

void vulkan_bind_pipeline(Render_Pipeline *pipeline) {
	vulkan_info.pipeline_layout = pipeline->layout; // to use when binding sets later
	vkCmdBindPipeline(VK_CMD(vulkan_info), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
}

void vulkan_bind_compute_pipeline(Render_Pipeline *pipeline) {
	vulkan_info.pipeline_layout = pipeline->layout; // to use when binding sets later
	vkCmdBindPipeline(VK_CMD(vulkan_info), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);
}

void vulkan_start_compute() {
	Vulkan_Frame *frame = &vulkan_info.frames[gfx.current_frame];
	
	// Compute submission
	vkWaitForFences(vulkan_info.device, 1, &frame->compute_in_flight_fence, VK_TRUE, UINT64_MAX);
	
	vkResetFences(vulkan_info.device, 1, &frame->compute_in_flight_fence);
	vulkan_info.active_command_buffer = &frame->compute_command_buffer;
	vkResetCommandBuffer(frame->compute_command_buffer, 0);

	if (vkBeginCommandBuffer(frame->compute_command_buffer, &vulkan_info.begin_info) != VK_SUCCESS) {
		logprint("vulkan_record_command_buffer()", "failed to begin recording command buffer\n");
	}	
}

void vulkan_end_compute() {
	Vulkan_Frame *frame = &vulkan_info.frames[gfx.current_frame];

	if (vkEndCommandBuffer(VK_CMD(vulkan_info)) != VK_SUCCESS) {
		logprint("vulkan_record_command_buffer()", "failed to record command buffer\n");
	}

	vulkan_info.compute_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vulkan_info.compute_submit_info.commandBufferCount = 1;
	vulkan_info.compute_submit_info.pCommandBuffers = &frame->compute_command_buffer;
	vulkan_info.compute_submit_info.signalSemaphoreCount = 0;
	vulkan_info.compute_submit_info.pSignalSemaphores = &frame->compute_finished_semaphore;

	if (vkQueueSubmit(vulkan_info.compute_queue, 1, &vulkan_info.compute_submit_info, frame->compute_in_flight_fence) != VK_SUCCESS) {
	    logprint("vulkan_end_compute()", "failed to submit compute command buffer!\n");
	};

	//vkWaitForFences(vulkan_info.device, 1, &vulkan_info.compute_in_flight_fences[gfx.current_frame], VK_TRUE, UINT64_MAX);
};

bool8 vulkan_start_frame(App_Window *window) {
	Vulkan_Frame *frame = &vulkan_info.frames[gfx.current_frame];

	vulkan_info.dynamic_buffer.offset = 0;
	
	if (gfx.current_frame == 0)
		vulkan_info.dynamic_uniform_buffer.offset = 0;

	frame->dynamic_offset_start = vulkan_info.dynamic_uniform_buffer.offset;

	if (window->resized) {
		vulkan_recreate_swap_chain(&vulkan_info, render_context.window_dim, global_assets);
		vulkan_info.draw_render_pass_info.renderArea.extent = vulkan_get_extent();
		vulkan_info.present_render_pass_info.renderArea.extent = vulkan_info.swap_chain_extent;

		window->resized = false;
	}

	// Waiting for the previous frame
	vkWaitForFences(vulkan_info.device, 1, &frame->in_flight_fence, VK_TRUE, UINT64_MAX);
	VkResult result = vkAcquireNextImageKHR(vulkan_info.device,
                                          vulkan_info.swap_chains[0],
                                          UINT64_MAX,
                                          frame->image_available_semaphore,
	                                        VK_NULL_HANDLE,
                                          &vulkan_info.image_index);

	// don't draw frame if swap chain is out of date
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		logprint("vulkan_start_frame()", "out of date swap chain\n");
		return 1;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		logprint("vulkan_start_frame()", "failed to acquire swap chain");
		return 1;
	}

	vulkan_info.draw_render_pass_info.framebuffer = vulkan_info.draw_framebuffers[vulkan_info.image_index];
	vulkan_info.present_render_pass_info.framebuffer = vulkan_info.swap_chain_framebuffers[vulkan_info.image_index];

	vkResetFences(vulkan_info.device, 1, &frame->in_flight_fence);
	vulkan_info.active_command_buffer = &frame->command_buffer;
	vkResetCommandBuffer(VK_CMD(vulkan_info), 0);
	
	if (vkBeginCommandBuffer(VK_CMD(vulkan_info), &vulkan_info.begin_info) != VK_SUCCESS) {
		logprint("vulkan_record_command_buffer()", "failed to begin recording command buffer\n");
	}	

	vkCmdBeginRenderPass(VK_CMD(vulkan_info), &vulkan_info.draw_render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	gfx.recording_frame = TRUE;
	return 0;
}

void vulkan_end_frame(Assets *assets, App_Window *window) {
	gfx.recording_frame = FALSE;
		;
	Vulkan_Frame *frame = &vulkan_info.frames[gfx.current_frame];
	frame->dynamic_offset_end = vulkan_info.dynamic_uniform_buffer.offset;

	vkCmdEndRenderPass(VK_CMD(vulkan_info));

		// Draw Render Pass
	if (render_context.resolution_scaling && !window->resized) {
		VkImageSubresourceLayers sub = {};
		sub.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sub.mipLevel = 0;
		sub.baseArrayLayer = 0;
		sub.layerCount = 1;

		VkImageBlit region = {};
		region.srcSubresource = sub;
		region.srcOffsets[1].x = render_context.resolution.x;
		region.srcOffsets[1].y = render_context.resolution.y;
		region.srcOffsets[1].z = 1;
		region.dstSubresource = sub;
		region.dstOffsets[1].x = vulkan_info.swap_chain_extent.width;
		region.dstOffsets[1].y = vulkan_info.swap_chain_extent.height;
		region.dstOffsets[1].z = 1;
		
		Vulkan_Texture *texture = &vulkan_info.swap_chain_textures[vulkan_info.image_index];
		vulkan_transition_image_layout(VK_CMD(vulkan_info), texture->image, texture->image_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);	
		vkCmdBlitImage(VK_CMD(vulkan_info), vulkan_info.draw_textures[vulkan_info.image_index].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);
		vulkan_transition_image_layout(VK_CMD(vulkan_info), texture->image, texture->image_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1);	
	}
	// End draw render pass

	if (vkEndCommandBuffer(VK_CMD(vulkan_info)) != VK_SUCCESS) {
		logprint("vulkan_record_command_buffer()", "failed to record command buffer\n");
	}

	VkSemaphore wait_semaphores[] = {
		frame->image_available_semaphore
	};
	vulkan_info.submit_info.pWaitSemaphores = wait_semaphores;
	vulkan_info.submit_info.pCommandBuffers = &frame->command_buffer;	
	vulkan_info.submit_info.pSignalSemaphores = &frame->render_finished_semaphore;

	if (vkQueueSubmit(vulkan_info.graphics_queue, 1, &vulkan_info.submit_info, frame->in_flight_fence) != VK_SUCCESS) {
		logprint("vulkan_draw_frame()", "failed to submit draw command buffer\n");
	}

	vulkan_info.present_info.pWaitSemaphores = &frame->render_finished_semaphore;
	vulkan_info.present_info.pImageIndices = &vulkan_info.image_index;
	
	VkResult result = vkQueuePresentKHR(vulkan_info.present_queue, &vulkan_info.present_info);

	// swap chain became out of date during drawing. sdl should have a window changed event
	// to flag the swap chain to update.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		logprint("vulkan_end_frame()", "out of date swap chain\n");
	} else if (result != VK_SUCCESS) {
		logprint("vulkan_end_frame()", "failed to acquire swap chain\n");
	}

	gfx.current_frame = (gfx.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//
// Mesh
//

void vulkan_init_mesh(Mesh *mesh) {
    Vulkan_Mesh *vulkan_mesh = (Vulkan_Mesh*)platform_malloc(sizeof(Vulkan_Mesh));

    u32 vertices_size = mesh->vertices_count * mesh->vertex_info.size;
    u32 indices_size = mesh->indices_count * sizeof(mesh->indices[0]);   
    u32 buffer_size = vertices_size + indices_size;

    void *memory = platform_malloc(buffer_size);

    memcpy(memory, (void*)mesh->vertices, vertices_size);
    memcpy((char*)memory + vertices_size, (void*)mesh->indices, indices_size);

		vulkan_mesh->vertices_offset = vulkan_get_next_offset(&vulkan_info.static_buffer.offset, buffer_size, VULKAN_STATIC_BUFFER_SIZE, false);

    vulkan_update_buffer(&vulkan_info, &vulkan_info.static_buffer.handle, &vulkan_info.static_buffer.memory, memory, buffer_size, vulkan_mesh->vertices_offset);
    vulkan_mesh->indices_offset = vulkan_mesh->vertices_offset + vertices_size;
    
    platform_free(memory);
    mesh->gpu_info = (void*)vulkan_mesh;
}

void vulkan_draw_mesh(Mesh *mesh) {
    Vulkan_Mesh *vulkan_mesh = (Vulkan_Mesh*)mesh->gpu_info;
    VkDeviceSize offsets[] = { vulkan_mesh->vertices_offset };
    vkCmdBindVertexBuffers(VK_CMD(vulkan_info), 0, 1, &vulkan_info.static_buffer.handle, offsets);
    vkCmdBindIndexBuffer(VK_CMD(vulkan_info), vulkan_info.static_buffer.handle, vulkan_mesh->indices_offset, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(VK_CMD(vulkan_info), mesh->indices_count, 1, 0, 0, 0);
}

void vulkan_depth_test(bool32 enable) {
	vkCmdSetDepthTestEnable(VK_CMD(vulkan_info), vulkan_bool(enable));
}

void vulkan_immediate_vertex(Vertex_XNU vertex) {
	float32 data[8];
	data[0] = vertex.position.x; data[1] = vertex.position.y; data[2] = vertex.position.z;
	data[3] = vertex.normal.x; data[4] = vertex.normal.y; data[5] = vertex.normal.z;
	data[6] = vertex.uv.x; data[7] = vertex.uv.y;
	vulkan_update_buffer(&vulkan_info, &vulkan_info.dynamic_buffer.handle, &vulkan_info.dynamic_buffer.memory, data, sizeof(Vertex_XNU), vulkan_info.dynamic_buffer.offset);
	vulkan_info.dynamic_buffer.offset += sizeof(Vertex_XNU);
}

void vulkan_immediate_vertex(Vertex_XU vertex) {
	float32 data[4];
	data[0] = vertex.position.x; data[1] = vertex.position.y; data[2] = vertex.uv.x; data[3] = vertex.uv.y;
	memcpy((char*)vulkan_info.dynamic_buffer.data + vulkan_info.dynamic_buffer.offset, data, sizeof(Vertex_XU));
	vulkan_info.dynamic_buffer.offset += sizeof(Vertex_XU);
}

void vulkan_draw_immediate(u32 vertices) {
  VkDeviceSize offsets[] = { vulkan_info.dynamic_buffer.offset - (vertices * sizeof(Vertex_XU)) };
  vkCmdBindVertexBuffers(VK_CMD(vulkan_info), 0, 1, &vulkan_info.dynamic_buffer.handle, offsets);
  vkCmdDraw(VK_CMD(vulkan_info), vertices, 1, 0, 0);
}

void test_draw_rect() {
    VkDeviceSize offsets[] = { vulkan_info.dynamic_buffer.offset };
    vulkan_immediate_vertex(Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, 1}, {0, 0} });
    vulkan_immediate_vertex(Vertex_XNU{ {-0.5, 0.5, 0}, {0, 0, 1}, {0, 1} });
    vulkan_immediate_vertex(Vertex_XNU{ {0.5, -0.5, 0}, {0, 0, 1}, {1, 0} });
    Object object = {};
    object.model = identity_m4x4();
    render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));
    vkCmdBindVertexBuffers(VK_CMD(vulkan_info), 0, 1, &vulkan_info.dynamic_buffer.handle, offsets);
    vkCmdDraw(VK_CMD(vulkan_info), 3, 1, 0, 0);
}

//
// Descriptor Sets
//

void vulkan_print_allocated_descriptors() {
	print("allocated descriptors:\n");
	print("uniform buffer: %d\n", vulkan_info.allocated_descriptors_uniform_buffer);
	print("sampler: %d\n", vulkan_info.allocated_descriptors_sampler);
	print("storage buffer: %d\n", vulkan_info.allocated_descriptors_storage_buffer);
	print("total: %d\n", vulkan_info.allocated_descriptors_uniform_buffer + vulkan_info.allocated_descriptors_sampler + vulkan_info.allocated_descriptors_storage_buffer);
}

void vulkan_allocate_descriptor_set(Layout *layout) {
	VkDescriptorSetLayout layouts[layout->max_sets];
	for (u32 i = 0; i < layout->max_sets; i++) {
		layouts[i] = layout->descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocate_info.descriptorPool = vulkan_info.descriptor_pool; // set in vulkan initialization function
	allocate_info.descriptorSetCount = layout->max_sets;
	allocate_info.pSetLayouts = layouts;

#if DEBUG

	for (u32 i = 0; i < 1; i++) {
		u32 *allocated = 0;
		
		switch(layout->bindings[i].descriptor_type) {
			case DESCRIPTOR_TYPE_UNIFORM_BUFFER: allocated = &vulkan_info.allocated_descriptors_uniform_buffer; break;
			case DESCRIPTOR_TYPE_SAMPLER: allocated = &vulkan_info.allocated_descriptors_sampler; break;
			case DESCRIPTOR_TYPE_STORAGE_BUFFER: allocated = &vulkan_info.allocated_descriptors_storage_buffer; break; 
		}

		if (allocated != 0)
			*allocated += layout->bindings[i].descriptor_count * layout->max_sets;
	}

#endif // DEBUG

	VkResult result = vkAllocateDescriptorSets(vulkan_info.device, &allocate_info, layout->descriptor_sets);
	if (result != VK_SUCCESS) {
		logprint("vulkan_allocate_descriptor_sets()", "failed to allocate descriptor sets (%s)\n", string_VkResult(result));
	}
}


internal void
vulkan_init_ubos(VkDescriptorSet *sets, Layout_Binding *layout_binding, u32 num_of_sets, u32 offsets[64]) {
	VkWriteDescriptorSet *write_sets = ARRAY_MALLOC(VkWriteDescriptorSet, num_of_sets);
	VkDescriptorBufferInfo *static_buffer_infos = ARRAY_MALLOC(VkDescriptorBufferInfo, (num_of_sets * layout_binding->descriptor_count));

	u32 alignment = (u32)vulkan_get_alignment(layout_binding->size * layout_binding->descriptor_count, (u32)vulkan_info.uniform_buffer_min_alignment);
	//u32 dyn_offset = vulkan_get_next_offset(&vulkan_info.dynamic_uniform_buffer.offset, alignment, VULKAN_DYNAMIC_UNIFORM_BUFFER_SIZE, false);

	VkDescriptorBufferInfo dynamic_buffer_info = {};
	dynamic_buffer_info.offset = 0;
  dynamic_buffer_info.range = layout_binding->size * layout_binding->descriptor_count;
	dynamic_buffer_info.buffer = vulkan_info.dynamic_uniform_buffer.handle;

	VkWriteDescriptorSet write_set = {};
	write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_set.dstBinding = layout_binding->binding;
	write_set.dstArrayElement = 0;
	write_set.descriptorType = vulkan_convert_descriptor_type(layout_binding->descriptor_type);
	write_set.descriptorCount = layout_binding->descriptor_count;
	write_set.pBufferInfo = &dynamic_buffer_info;

	// set up static buffer info if regular uniform buffer
	if (layout_binding->descriptor_type == DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
		for (u32 i = 0; i < num_of_sets; i++) {
			u32 offset = vulkan_get_next_offset(&vulkan_info.static_uniform_buffer.offset, alignment, VULKAN_STATIC_UNIFORM_BUFFER_SIZE, false);
			offsets[i] = offset;
		}

		for (u32 i = 0; i < num_of_sets * layout_binding->descriptor_count; i++) {
			u32 offset_index = i / layout_binding->descriptor_count;

			VkDescriptorBufferInfo static_buffer_info = {};
			static_buffer_info.offset = offsets[offset_index];
  		static_buffer_info.range = layout_binding->size;
			static_buffer_info.buffer = vulkan_info.static_uniform_buffer.handle;

			static_buffer_infos[i] = static_buffer_info;
		}	

	}

	for (u32 i = 0; i < num_of_sets; i++) {
		write_sets[i] = write_set;
		write_sets[i].dstSet = sets[i];
		if (layout_binding->descriptor_type == DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			write_sets[i].pBufferInfo = &static_buffer_infos[i * layout_binding->descriptor_count];
		}
	}

	vkUpdateDescriptorSets(vulkan_info.device, num_of_sets, write_sets, 0, nullptr);

	platform_free(write_sets);
	platform_free(static_buffer_infos);
}

internal void
vulkan_init_bitmaps(VkDescriptorSet *sets, u32 num_of_sets, Bitmap *bitmap, Layout_Binding *layout_binding) {
	Vulkan_Texture *texture = (Vulkan_Texture *)bitmap->gpu_info;
	VkWriteDescriptorSet *write_sets = ARRAY_MALLOC(VkWriteDescriptorSet, num_of_sets);

	VkDescriptorImageInfo image_info[TEXTURE_ARRAY_SIZE] = {};
	for (u32 i = 0; i < TEXTURE_ARRAY_SIZE; i++) {
		image_info[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info[i].imageView = texture->image_view;
		image_info[i].sampler = texture->sampler;
	}

	VkWriteDescriptorSet write_set = {};
	write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_set.dstBinding = layout_binding->binding;
	write_set.dstArrayElement = 0;
	write_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_set.descriptorCount = TEXTURE_ARRAY_SIZE;
	write_set.pImageInfo = image_info;

	for (u32 i = 0; i < num_of_sets; i++) {
		write_sets[i] = write_set;
		write_sets[i].dstSet = sets[i];
	}

  vkUpdateDescriptorSets(vulkan_info.device, num_of_sets, write_sets, 0, nullptr);

	platform_free(write_sets);
}

void vulkan_init_layout_offsets(Layout *layout, Bitmap *bitmap) {
	if (layout->bindings[0].descriptor_type == DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
      layout->bindings[0].descriptor_type == DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
		vulkan_init_ubos(layout->descriptor_sets, &layout->bindings[0], layout->max_sets, layout->offsets);
	}

	if (layout->bindings[0].descriptor_type == DESCRIPTOR_TYPE_SAMPLER) {
		vulkan_init_bitmaps(layout->descriptor_sets, layout->max_sets, bitmap, &layout->bindings[0]);
	}
}

void vulkan_create_set_layout(Layout *layout) {

	if (layout->bindings_count = 0) {
		logprint("(vulkan) vulkan_create_set_layout()", "no bindings set\n");
	}

	// @TODO just creating with one binding right now
	if (layout->bindings_count > 1) {
		logprint("(vulkan) vulkan_create_set_layout()", "only supporting 1 binding right now\n");
		return;
	}

	VkDescriptorSetLayoutBinding vulkan_binding = {};
	vulkan_binding.binding = layout->bindings[0].binding;
	vulkan_binding.descriptorType = vulkan_convert_descriptor_type(layout->bindings[0].descriptor_type);
 	vulkan_binding.descriptorCount = layout->bindings[0].descriptor_count;
	for (u32 i = 0; i < layout->bindings[0].stages_count; i++) {
		vulkan_binding.stageFlags |= vulkan_convert_shader_stage(layout->bindings[0].stages[i]);
	}
	vulkan_binding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1;
	layout_info.pBindings = &vulkan_binding;
	
	if (vkCreateDescriptorSetLayout(vulkan_info.device, &layout_info, nullptr, &layout->descriptor_set_layout) != VK_SUCCESS) {
		logprint("vulkan_create_descriptor_set_layout()", "failed to create descriptor set layout\n");
	}

}


// Descriptor Set Usage

Descriptor vulkan_get_descriptor_set(Layout *layout) {
	if (layout == 0) {
		logprint("vulkan_get_descriptor_set()", "layout is null\n");
		ASSERT(0);
	}
	
  if (layout->sets_in_use + 1 > layout->max_sets / 2) {
		logprint("vulkan_get_descriptor_set()", "used all sets\n");
		ASSERT(0);
  }

  u32 return_index = layout->sets_in_use++;
  if (gfx.current_frame == 1)
    return_index += layout->max_sets / 2;

	Descriptor desc = {};
	desc.binding = layout->bindings[0];
	desc.offset = layout->offsets[return_index];
	desc.set_number = layout->set_number;
	desc.vulkan_set = &layout->descriptor_sets[return_index];

  return desc;
}

Descriptor vulkan_get_descriptor_set(u32 layout_id) {
	if (!gfx.recording_frame) {
		// if we are not in the middle of recording commands for a frame, don't check active
		// shader for the layout.
		return vulkan_get_descriptor_set(&gfx.layouts[layout_id]);
	}

	Shader *shader = &global_assets->data[gfx.active_shader_id].shader;
	Layout_Set *set = &shader->set;
	
	for (u32 i = 0; i < set->layouts_count; i++) {
		if (set->layouts[i]->id == layout_id) {
			return vulkan_get_descriptor_set(set->layouts[i]);
		}
	}
		
	logprint("vulkan_get_descriptor_set()", "no layout in set with id in shader: %d\n", gfx.active_shader_id);
	ASSERT(0);
	return {};
}

Descriptor vulkan_get_descriptor_set_index(u32 layout_id, u32 return_index) {
	Layout *layout = &gfx.layouts[layout_id];
	
	if (layout->sets_in_use + 1 > layout->max_sets)
	    ASSERT(0);

	if (return_index < layout->sets_in_use) {
		logprint("vulkan_get_descriptor_set()", "descriptor could already be in use\n");
	} else {
		layout->sets_in_use = return_index + 1; // jump to after this set
	}

	Descriptor desc = {};
	desc.binding = layout->bindings[0];
	desc.offset = layout->offsets[return_index];
	desc.set_number = layout->set_number;
	desc.vulkan_set = &layout->descriptor_sets[return_index];

	return desc;
}


void vulkan_update_ubo(Descriptor desc, void *data) {
	if (desc.binding.descriptor_type != DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
		logprint("vulkan_update_ubo()", "tried to update non static buffer in static way\n");
		return;
	}

	memcpy((char*)vulkan_info.static_uniform_buffer.data + desc.offset, data, desc.binding.size);
}

/*
IMPORTANT:
This coming after the update of the descriptor is very important in scenarios where the data
is changing frame to frame.
Maybe should combine update and bind functions.
*/
void vulkan_bind_descriptor_set(Descriptor desc) {
	if (desc.binding.stages[0] == SHADER_STAGE_COMPUTE)
		vkCmdBindDescriptorSets(VK_CMD(vulkan_info), VK_PIPELINE_BIND_POINT_COMPUTE, vulkan_info.pipeline_layout, desc.set_number, 1, desc.vulkan_set, 0, nullptr);
	else
		vkCmdBindDescriptorSets(VK_CMD(vulkan_info), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_info.pipeline_layout, desc.set_number, 1, desc.vulkan_set, 0, nullptr);
}

void vulkan_bind_descriptor_sets(Descriptor desc, void *data) {
	u32 alignment = (u32)vulkan_get_alignment(desc.binding.size, (u32)vulkan_info.uniform_buffer_min_alignment);

	u32 next_frame_index = gfx.current_frame + 1;
	if (next_frame_index >= MAX_FRAMES_IN_FLIGHT)
		next_frame_index = 0;
	
	u32 offset = vulkan_get_next_offset(&vulkan_info.dynamic_uniform_buffer.offset, alignment, VULKAN_DYNAMIC_UNIFORM_BUFFER_SIZE, true);

	if (offset >= vulkan_info.frames[next_frame_index].dynamic_offset_start && offset < vulkan_info.frames[next_frame_index].dynamic_offset_end) {
		vulkan_info.dynamic_uniform_buffer.offset = vulkan_get_next_offset(&vulkan_info.frames[next_frame_index].dynamic_offset_end, alignment, VULKAN_DYNAMIC_UNIFORM_BUFFER_SIZE, true);
		offset = vulkan_info.dynamic_uniform_buffer.offset;
	}

	memcpy((char*)vulkan_info.dynamic_uniform_buffer.data + offset, data, desc.binding.size);

	vkCmdBindDescriptorSets(VK_CMD(vulkan_info), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_info.pipeline_layout, desc.set_number, 1, desc.vulkan_set, 1, &offset);
}

void vulkan_push_constants(u32 shader_stage, void *data, u32 data_size) {
	vkCmdPushConstants(VK_CMD(vulkan_info), vulkan_info.pipeline_layout, vulkan_convert_shader_stage(shader_stage), 0, data_size, data);
}

u32 vulkan_set_texture(Descriptor *desc, void *gpu_handle) {
	Vulkan_Texture *texture = (Vulkan_Texture *)gpu_handle;
	
	VkDescriptorImageInfo image_info = {};
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image_info.imageView = texture->image_view;
	image_info.sampler = texture->sampler;

	VkWriteDescriptorSet descriptor_writes[1] = {};

	descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_writes[0].dstSet = *desc->vulkan_set;
	descriptor_writes[0].dstBinding = desc->binding.binding;
	descriptor_writes[0].dstArrayElement = desc->texture_index;
	descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_writes[0].descriptorCount = 1;
	descriptor_writes[0].pImageInfo = &image_info;

	vkUpdateDescriptorSets(vulkan_info.device, ARRAY_COUNT(descriptor_writes), descriptor_writes, 0, nullptr);

	u32 index = desc->texture_index;

	desc->texture_index++;

	return index;
}

u32 vulkan_set_bitmap(Descriptor *desc, Bitmap *bitmap) {
	return vulkan_set_texture(desc, bitmap->gpu_info);
}

void vulkan_set_bitmap(Descriptor *desc, Bitmap *bitmaps, u32 bitmaps_count) {
	VkDescriptorImageInfo image_infos[60] = {};
	
	for (u32 i = 0; i < bitmaps_count; i++) {
		Vulkan_Texture *texture = (Vulkan_Texture *)bitmaps[i].gpu_info;
		
		image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_infos[i].imageView = texture->image_view;
		image_infos[i].sampler = texture->sampler;
	}

	VkWriteDescriptorSet descriptor_writes[1] = {};
	descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_writes[0].dstSet = *desc->vulkan_set;
	descriptor_writes[0].dstBinding = desc->binding.binding;
	descriptor_writes[0].dstArrayElement = desc->texture_index;
	descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_writes[0].descriptorCount = 1;
	descriptor_writes[0].pImageInfo = image_infos;

	vkUpdateDescriptorSets(vulkan_info.device, ARRAY_COUNT(descriptor_writes), descriptor_writes, 0, nullptr);

	u32 index = desc->texture_index;

	desc->texture_index += bitmaps_count;
}

internal void
vulkan_set_storage_buffer1(Descriptor desc, u32 size) {
	VkDescriptorBufferInfo buffer_info = {};
	buffer_info.offset = 0;
	buffer_info.range = size;
	buffer_info.buffer = vulkan_info.storage_buffer.handle;

	VkWriteDescriptorSet write_set = {};
	write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_set.dstSet = *desc.vulkan_set;
	write_set.dstBinding = desc.binding.binding;
	write_set.dstArrayElement = 0;
	write_set.descriptorType = vulkan_convert_descriptor_type(desc.binding.descriptor_type);
	write_set.descriptorCount = 1;
	write_set.pBufferInfo = &buffer_info;

	vkUpdateDescriptorSets(vulkan_info.device, 1, &write_set, 0, nullptr);
}

internal void
vulkan_set_storage_buffer(Descriptor desc, u32 size) {
	VkDescriptorBufferInfo buffer_info = {};
	buffer_info.offset = 0;
    buffer_info.range = size;
	buffer_info.buffer = vulkan_info.triangle_buffer.handle;

	VkWriteDescriptorSet write_set = {};
    write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_set.dstSet = *desc.vulkan_set;
    write_set.dstBinding = desc.binding.binding;
    write_set.dstArrayElement = 0;
    write_set.descriptorType = vulkan_convert_descriptor_type(desc.binding.descriptor_type);
    write_set.descriptorCount = 1;
    write_set.pBufferInfo = &buffer_info;
	
	vkUpdateDescriptorSets(vulkan_info.device, 1, &write_set, 0, nullptr);
}

internal void
vulkan_update_for_anti_aliasing() {
	if (render_context.anti_aliasing)
		vulkan_info.msaa_samples = vulkan_get_max_usable_sample_count();
	else
		vulkan_info.msaa_samples = VK_SAMPLE_COUNT_1_BIT;

	vkDestroyRenderPass(vulkan_info.device, vulkan_info.draw_render_pass, nullptr);
	vulkan_create_draw_render_pass(&vulkan_info);
	vulkan_create_draw_framebuffers(&vulkan_info);
}
