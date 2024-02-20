//
// Vulkan Debug
//

internal VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
	print("validation layer: %s\n", callback_data->pMessage);
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

internal void
vulkan_create_instance(Vulkan_Info *vulkan_info, const char **instance_extensions, u32 instance_extensions_count) {
    VkInstance instance;

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

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
	}

	vulkan_info->instance = instance;
}


internal Vulkan_Queue_Family_Indices
vulkan_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
	Vulkan_Queue_Family_Indices indices;

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	VkQueueFamilyProperties *queue_families = ARRAY_MALLOC(VkQueueFamilyProperties, queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

	for (u32 queue_index = 0; queue_index < queue_family_count; queue_index++) {
		if (queue_families[queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family_found = true;
			indices.graphics_family = queue_index;
		}

		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_index, surface, &present_support);
		if (present_support) {
			indices.present_family_found = true;
			indices.present_family = queue_index;
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
	/*
	VkPhysicalDeviceProperties device_properties;
	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceProperties(device, &device_properties);
	vkGetPhysicalDeviceFeatures(device, &device_features);

	return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader;
	*/

	Vulkan_Queue_Family_Indices indices = vulkan_find_queue_families(device, surface);

	bool8 extensions_supported = vulkan_check_device_extension_support(device, device_extensions, device_extensions_count);
	bool8 swap_chain_adequate = false;
	if (extensions_supported) {
		Vulkan_Swap_Chain_Support_Details swap_chain_support = vulkan_query_swap_chain_support(device, surface);
		swap_chain_adequate = swap_chain_support.formats_count && swap_chain_support.present_modes_count;
		platform_free(swap_chain_support.formats);
		platform_free(swap_chain_support.present_modes);
	}

	VkPhysicalDeviceFeatures supported_features;
	vkGetPhysicalDeviceFeatures(device, &supported_features);

	return indices.graphics_family_found && extensions_supported && swap_chain_adequate && supported_features.samplerAnisotropy;
}

internal void
vulkan_pick_physical_device(Vulkan_Info *info) {
	info->physical_device = VK_NULL_HANDLE;

	u32 device_count = 0;
	vkEnumeratePhysicalDevices(info->instance, &device_count, nullptr);

	if (device_count == 0)
		logprint("vulkan_pick_physical_device()", "failed to find GPUs with Vulkan support");

	VkPhysicalDevice *devices = ARRAY_MALLOC(VkPhysicalDevice, device_count);
	vkEnumeratePhysicalDevices(info->instance, &device_count, devices);

	for (u32 device_index = 0; device_index < device_count; device_index++) {
		if (vulkan_is_device_suitable(devices[device_index], info->surface, info->device_extensions, ARRAY_COUNT(info->device_extensions))) {
			info->physical_device = devices[device_index];
			break;
		}
	}

	platform_free(devices);

	if (info->physical_device == VK_NULL_HANDLE) {
		logprint("vulkan_pick_physical_device()", "failed to find a suitable GPU\n");
	}
}

internal void
vulkan_create_logical_device(Vulkan_Info *info) {
	Vulkan_Queue_Family_Indices indices = vulkan_find_queue_families(info->physical_device, info->surface);

	// Specify the device queues we want
	VkDeviceQueueCreateInfo *queue_create_infos = ARRAY_MALLOC(VkDeviceQueueCreateInfo, indices.unique_families);
	u32 unique_queue_families[indices.unique_families] = { indices.graphics_family, indices.present_family };

	float32 queue_priority = 1.0f;
	for (u32 queue_index = 0; queue_index < indices.unique_families; queue_index++) {
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

	// Set up device
	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_create_infos;
	create_info.queueCreateInfoCount = indices.unique_families;
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
	vkGetDeviceQueue(info->device, indices.graphics_family, 0, &info->graphics_queue);
	vkGetDeviceQueue(info->device, indices.present_family, 0, &info->present_queue);

	platform_free(queue_create_infos);
}

// Where SRGB is turned on
internal VkSurfaceFormatKHR
vulkan_choose_swap_surface_format(VkSurfaceFormatKHR *formats, u32 count) {
	for (u32 i = 0; i < count; i++) {
		if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return formats[i];
		}
	}

	return formats[0];
}

// VSYNC OFF : VK_PRESENT_MODE_IMMEDIATE_KHR 
// VSYNC ON  : VK_PRESENT_MODE_MAILBOX_KHR
internal VkPresentModeKHR
vulkan_choose_swap_present_mode(VkPresentModeKHR *modes, u32 count) {
	for (u32 i = 0; i < count; i++) {
		if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
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
vulkan_create_swap_chain(Vulkan_Info *info) {
	Vulkan_Swap_Chain_Support_Details swap_chain_support = vulkan_query_swap_chain_support(info->physical_device, info->surface);

	VkSurfaceFormatKHR surface_format = vulkan_choose_swap_surface_format(swap_chain_support.formats, swap_chain_support.formats_count);
	VkPresentModeKHR present_mode = vulkan_choose_swap_present_mode(swap_chain_support.present_modes, swap_chain_support.present_modes_count);
	VkExtent2D extent = vulkan_choose_swap_extent(swap_chain_support.capabilities, info->window_width, info->window_height);

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
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	Vulkan_Queue_Family_Indices indices = vulkan_find_queue_families(info->physical_device, info->surface);
	u32 queue_family_indices[2] = { indices.graphics_family, indices.present_family };

	if (indices.graphics_family != indices.present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = ARRAY_COUNT(queue_family_indices);
		create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0; // optional
		create_info.pQueueFamilyIndices = nullptr; // optional
	}

	create_info.preTransform = swap_chain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(info->device, &create_info, nullptr, &info->swap_chains[0])) {
		logprint("vulkan_create_swap_chain()", "failed to create swap chain\n");
	}
	
	u32 swap_chain_images_count = 0;
	vkGetSwapchainImagesKHR(info->device, info->swap_chains[0], &swap_chain_images_count, nullptr);
	info->swap_chain_images.resize(swap_chain_images_count);
	vkGetSwapchainImagesKHR(info->device, info->swap_chains[0], &swap_chain_images_count, info->swap_chain_images.get_data());

	info->swap_chain_image_format = surface_format.format;
	info->swap_chain_extent = extent;

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

internal void
vulkan_create_render_pass(Vulkan_Info *info) {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format         = info->swap_chain_image_format;
	color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment = {};
	depth_attachment.format         = vulkan_find_depth_format(info->physical_device);
	depth_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount    = 1;
	subpass.pColorAttachments       = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;
	
	// Subpass dependencies
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	
	VkAttachmentDescription attachments[] = { color_attachment, depth_attachment };

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = ARRAY_COUNT(attachments);
	render_pass_info.pAttachments = attachments;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	if (vkCreateRenderPass(info->device, &render_pass_info, nullptr, &info->render_pass) != VK_SUCCESS) {
		logprint("vulkan_create_render_pass()", "failed to create render pass\n");
	}
}

internal void
vulkan_create_frame_buffers(Vulkan_Info *info) {
	info->swap_chain_framebuffers.resize(info->swap_chain_image_views.get_size());
	for (u32 i = 0; i < info->swap_chain_image_views.get_size(); i++) {
		VkImageView attachments[] = {
			info->swap_chain_image_views[i],
			info->depth_image_view
		};
	
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	    framebuffer_info.renderPass      = info->render_pass;
	    framebuffer_info.attachmentCount = ARRAY_COUNT(attachments);
	    framebuffer_info.pAttachments    = attachments;
	    framebuffer_info.width           = info->swap_chain_extent.width;
	    framebuffer_info.height          = info->swap_chain_extent.height;
	    framebuffer_info.layers          = 1;

		if (vkCreateFramebuffer(info->device, &framebuffer_info, nullptr, &info->swap_chain_framebuffers[i]) != VK_SUCCESS) {
			logprint("vulkan_create_frame_buffers()", "failed to create framebuffer\n");
		}
	}
}

internal void
vulkan_create_command_pool(Vulkan_Info *info) {
	Vulkan_Queue_Family_Indices queue_family_indices = vulkan_find_queue_families(info->physical_device, info->surface);
	
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = queue_family_indices.graphics_family;

	if (vkCreateCommandPool(info->device, &pool_info, nullptr, &info->command_pool) != VK_SUCCESS) {
		logprint("vulkan_create_command_pool()", "failed to create command pool\n");
	}
}

internal void
vulkan_create_command_buffers(Vulkan_Info *info) {
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = info->command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = info->MAX_FRAMES_IN_FLIGHT;

	if (vkAllocateCommandBuffers(info->device, &alloc_info, info->command_buffers) != VK_SUCCESS) {
		logprint("vulkan_create_command_buffer()", "failed to allocate command buffers\n");
	}
}

internal void
vulkan_create_sync_objects(Vulkan_Info *info) {
	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (u32 i = 0; i < info->MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(info->device, &semaphore_info, nullptr, &info->image_available_semaphore[i]) != VK_SUCCESS ||
	    	vkCreateSemaphore(info->device, &semaphore_info, nullptr, &info->render_finished_semaphore[i]) != VK_SUCCESS ||
	    	vkCreateFence    (info->device, &fence_info,     nullptr, &info->in_flight_fence[i]          ) != VK_SUCCESS) {
	    	logprint("vulkan_create_sync_objects()", "failed to create semaphores\n");
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

//
// Descriptor Sets
//

internal VkDeviceSize
vulkan_get_alignment(VkDeviceSize in, u32 alignment) {
	while(in % alignment != 0) {
		in++;
	}
	return in;
}

internal void
vulkan_create_descriptor_set(Shader *shader, u32 descriptor_set_count, u32 pool_index, VkDescriptorSet *sets) {
	Vulkan_Shader_Info *set_info = &shader->vulkan_infos[pool_index];

	// Create descriptor sets
	VkDescriptorSetLayout layouts[set_info->max_sets * vulkan_info.MAX_FRAMES_IN_FLIGHT];
	for (u32 i = 0; i < descriptor_set_count; i++) {
		layouts[i] = set_info->descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocate_info.descriptorPool = shader->vulkan_infos[pool_index].descriptor_pool;
	allocate_info.descriptorSetCount = descriptor_set_count;
	allocate_info.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(vulkan_info.device, &allocate_info, sets) != VK_SUCCESS) {
		logprint("vulkan_create_descriptor_sets()", "failed to allocate descriptor sets\n");
	}
}

#define VULKAN_STATIC_BUFFER_SIZE 20000000

internal u32
vulkan_get_next_offset(u32 *offset, u32 in_data_size) {
	u32 return_offset = *offset;
    *offset += in_data_size;
	if (*offset > VULKAN_STATIC_BUFFER_SIZE) {
		ASSERT(0);
		//*offset = 0;
		//return *offset;
	}
	return return_offset;
}

Descriptor_Set*
vulkan_get_descriptor_set(Shader *shader, bool8 layout_index) {

	u32 next_set = shader->vulkan_infos[layout_index].sets_count++;

	if (next_set > shader->vulkan_infos[layout_index].max_sets) {
		logprint("vulkan_get_descriptor_set()", "ran out of sets to use in shader\n");
		ASSERT(0);
		return 0;
	}

	return &shader->descriptor_sets[layout_index][next_set];
}

internal void
vulkan_init_descriptor_ub(Descriptor_Set *set, Descriptor *descriptor, u32 size, u32 binding) {
	VkDescriptorSet **vulkan_set = (VkDescriptorSet **)set->gpu_info;
	
	descriptor->size = size;
	descriptor->binding = binding;	

	// because two frames can be in flight
	u32 offset = (u32)vulkan_get_alignment(size, (u32)vulkan_info.uniform_buffer_min_alignment);
	u32 buffer_size = (u32)vulkan_get_alignment(offset + size, (u32)vulkan_info.uniform_buffer_min_alignment);

	descriptor->offsets[0] = vulkan_get_next_offset(&vulkan_info.dymanic_uniforms_offset, buffer_size);
	descriptor->offsets[1] = (u32)vulkan_get_alignment(descriptor->offsets[0] + size, (u32)vulkan_info.uniform_buffer_min_alignment);
	
	for (u32 i = 0; i < vulkan_info.MAX_FRAMES_IN_FLIGHT; i++) {
	    VkDescriptorBufferInfo buffer_info = {};
	    buffer_info.buffer = vulkan_info.uniform_buffer;
	    buffer_info.offset = descriptor->offsets[i];
	    buffer_info.range = descriptor->size;

	    VkWriteDescriptorSet descriptor_write = {};
	    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	    descriptor_write.dstSet = *(vulkan_set[i]);
	    descriptor_write.dstBinding = descriptor->binding;
	    descriptor_write.dstArrayElement = 0;
	    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	    descriptor_write.descriptorCount = 1;
	    descriptor_write.pBufferInfo = &buffer_info;

	    vkUpdateDescriptorSets(vulkan_info.device, 1, &descriptor_write, 0, nullptr);
	}
}

internal u32
vulkan_convert_descriptor_type(u32 descriptor_type) {
	switch(descriptor_type) {
		case DESCRIPTOR_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DESCRIPTOR_TYPE_SAMPLER:        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		default: {
			logprint("vulkan_convert_descriptor_type()", "%d is not a render descriptor type\n", descriptor_type);
			return 0;
		}
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
		default: {
			logprint("vulkan_convert_shader_stage()", "%d is not a render shader stage\n", shader_stage);
			return 0;
		}
	};
}

internal VkDescriptorSetLayoutBinding
vulkan_create_descriptor(Descriptor descriptor) {
	VkDescriptorSetLayoutBinding layout_binding = {};
	layout_binding.binding = descriptor.binding;
    layout_binding.descriptorType = (VkDescriptorType)vulkan_convert_descriptor_type(descriptor.type);
    layout_binding.descriptorCount = 1;
	layout_binding.pImmutableSamplers = nullptr; // Optional
	for (u32 i = 0; i < descriptor.stages_count; i++) {
		layout_binding.stageFlags |= vulkan_convert_shader_stage(descriptor.stages[i]);
	}
	return layout_binding;
}

// descriptor_set_count is how many descripotr sets to allocate
internal void
vulkan_create_descriptor_pool(Shader *shader, u32 max_sets, u32 set_index) {
	Descriptor_Set *set = &shader->layout_sets[set_index]; // passing the layout that was defined for the set

	VkDescriptorSetLayoutBinding bindings[set->max_descriptors] = {};
	for (u32 i = 0; i < set->descriptors_count; i++) {
		bindings[i] = vulkan_create_descriptor(set->descriptors[i]);
	}

	VkDescriptorSetLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = set->descriptors_count;
	layout_info.pBindings = bindings;
	
	if (vkCreateDescriptorSetLayout(vulkan_info.device, &layout_info, nullptr, &shader->vulkan_infos[set_index].descriptor_set_layout) != VK_SUCCESS) {
		logprint("vulkan_create_descriptor_set_layout()", "failed to create descriptor set layout\n");
	}

	VkDescriptorPoolSize pool_sizes[ARRAY_COUNT(bindings)] = {};
	for (u32 i = 0; i < set->descriptors_count; i++) {
		pool_sizes[i].type = bindings[i].descriptorType;
		pool_sizes[i].descriptorCount = max_sets * vulkan_info.MAX_FRAMES_IN_FLIGHT; // how many of that type to allocate
	}

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = pool_sizes;
	pool_info.maxSets = max_sets * vulkan_info.MAX_FRAMES_IN_FLIGHT;

	if (vkCreateDescriptorPool(vulkan_info.device, &pool_info, nullptr, &shader->vulkan_infos[set_index].descriptor_pool) != VK_SUCCESS) {
		logprint("vulkan_crate_descriptor_pool()", "failed to create descriptor pool\n");
	}

	//
	// setting up shader descriptor set buffer/cache
	//

	vulkan_create_descriptor_set(shader, shader->vulkan_infos[set_index].max_sets * 2, set_index, shader->vulkan_infos[set_index].descriptor_sets);

	u32 temp_index = 0;
	for (u32 i = 0; i < shader->vulkan_infos[set_index].max_sets; i++) {

		shader->descriptor_sets[set_index][i].gpu_info[0] = &shader->vulkan_infos[set_index].descriptor_sets[shader->vulkan_infos[set_index].sets_count++];
		shader->descriptor_sets[set_index][i].gpu_info[1] = &shader->vulkan_infos[set_index].descriptor_sets[shader->vulkan_infos[set_index].sets_count++];

		for (u32 j = 0; j < shader->layout_sets[set_index].descriptors_count; j++) {
			if (shader->layout_sets[set_index].descriptors[j].type == DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				vulkan_init_descriptor_ub(&shader->descriptor_sets[set_index][i], 
										  &shader->descriptor_sets[set_index][i].descriptors[j], 
										   shader->layout_sets[set_index].descriptors[j].size,
										   shader->layout_sets[set_index].descriptors[j].binding);
		}
	}
	shader->vulkan_infos[set_index].sets_count = 0;
}

void vulkan_bind_descriptor_set(Descriptor_Set *set, u32 first_set) {
	VkDescriptorSet *vulkan_set = (VkDescriptorSet *)set->gpu_info[vulkan_info.current_frame];
	//u32 first_set = 0;
	vkCmdBindDescriptorSets(vulkan_active_cmd_buffer(&vulkan_info), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_info.pipeline_layout, first_set, 1, vulkan_set, 0, nullptr);
}

// updates the ubo memory
// static update means that it fills in the memory for all of the frames in flight
internal void
vulkan_update_ubo(Descriptor_Set *set, u32 descriptor_index, void *data, bool8 static_update) {

	Descriptor *descriptor = &set->descriptors[descriptor_index];

	if (static_update) { // update all frames
		for (u32 i = 0; i < vulkan_info.MAX_FRAMES_IN_FLIGHT; i++) {
			memcpy((char*)vulkan_info.uniform_data + descriptor->offsets[i], data, descriptor->size);
		}
	} else {
		memcpy((char*)vulkan_info.uniform_data + descriptor->offsets[vulkan_info.current_frame], data, descriptor->size);
	}
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

internal void
vulkan_create_graphics_pipeline(Render_Pipeline *pipeline, Vertex_Info vertex_info) {
	Shader *shader = pipeline->shader;

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

	// Setting up layouts
	VkDescriptorSetLayout layouts[2];
	for (u32 i = 0; i < 2; i++) {
		layouts[i] = shader->vulkan_infos[i].descriptor_set_layout;
	}

	Descriptor_Set *push_set = &shader->layout_sets[2];
	u32 push_constant_count = push_set->descriptors_count;
	VkPushConstantRange push_constant_ranges[push_set->max_descriptors] = {};
	for (u32 i = 0; i < push_constant_count; i++) {
		VkPushConstantRange range = {};
		range.stageFlags = vulkan_convert_shader_stage(push_set->descriptors[i].stages[0]);
		range.offset = push_set->descriptors[i].offsets[0];
		range.size = push_set->descriptors[i].size;
		push_constant_ranges[i] = range;
	}

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount         = 2;        // Optional
	pipeline_layout_info.pSetLayouts            = layouts;  // Optional
	pipeline_layout_info.pushConstantRangeCount = push_constant_count;  // Optional
	pipeline_layout_info.pPushConstantRanges    = push_constant_ranges; // Optional

	if (vkCreatePipelineLayout(vulkan_info.device, &pipeline_layout_info, nullptr, &pipeline->pipeline_layout) != VK_SUCCESS) {
		logprint("vulkan_create_graphics_pipeline()", "failed to create pipeline layout\n");
	}

	VkDynamicState dynamic_states[] = { 
		VK_DYNAMIC_STATE_VIEWPORT, 
		VK_DYNAMIC_STATE_SCISSOR 
	};
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = ARRAY_COUNT(dynamic_states);
	dynamic_state.pDynamicStates = dynamic_states;

	// Create the pipeline vertex input info
	u32 vertex_size = 0;
	for (u32 i = 0; i < vertex_info.attributes_count; i++) {
		switch(vertex_info.formats[i]) {
			case VECTOR2: vertex_size += sizeof(Vector2); break;
			case VECTOR3: vertex_size += sizeof(Vector3); break;
		}
	}

	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = vertex_size;
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	
	VkVertexInputAttributeDescription attribute_descriptions[5] = {}; // 5 = vertex max attributes
	for (u32 i = 0; i < vertex_info.attributes_count; i++) {
		attribute_descriptions[i].binding = 0;
		attribute_descriptions[i].location = i;
		attribute_descriptions[i].format = convert_to_vulkan(vertex_info.formats[i]);
		attribute_descriptions[i].offset = vertex_info.offsets[i];
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount   = 1;
	vertex_input_info.pVertexBindingDescriptions      = &binding_description;         // Optional
	vertex_input_info.vertexAttributeDescriptionCount = vertex_info.attributes_count;
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
	multisampling.sampleShadingEnable   = VK_FALSE;
	multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading      = 1.0f;                  // Optional
	multisampling.pSampleMask           = nullptr;               // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE;              // Optional
	multisampling.alphaToOneEnable      = VK_FALSE; 			 // Optional

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable         = VK_TRUE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
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
	pipeline_create_info.layout              = pipeline->pipeline_layout;
	pipeline_create_info.renderPass          = vulkan_info.render_pass;
	pipeline_create_info.subpass             = 0;
	pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE;        // Optional
	pipeline_create_info.basePipelineIndex   = -1;                    // Optional

	if (vkCreateGraphicsPipelines(vulkan_info.device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline->graphics_pipeline) != VK_SUCCESS) {
		logprint("vulkan_create_graphics_pipeline()", "failed to create graphics pipelines\n");
	}

	for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
		if (shader_modules[i] != 0)
			vkDestroyShaderModule(vulkan_info.device, shader_modules[i], nullptr);
	}
}

//
// Images
//

internal void
vulkan_create_image(VkDevice device, VkPhysicalDevice physical_device, u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &image_memory) {
	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
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
vulkan_transition_image_layout(Vulkan_Info *info, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) {
	VkCommandBuffer command_buffer = vulkan_begin_single_time_commands(info->device, info->command_pool);
	
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
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
	} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
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

	vulkan_end_single_time_commands(command_buffer, info->device, info->command_pool, info->graphics_queue);
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
vulkan_create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags) {
	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect_flags;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
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
	VkFormat depth_format = vulkan_find_depth_format(info->physical_device);
	vulkan_create_image(info->device, info->physical_device, info->swap_chain_extent.width, info->swap_chain_extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, info->depth_image, info->depth_image_memory);
	info->depth_image_view = vulkan_create_image_view(info->device, info->depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
	vulkan_transition_image_layout(info, info->depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);	
}

// TODO: Make it so that images can be created asynchronously (vulkan-tutorial = Texture mapping/Images)

//
// Texture images
//

internal void
vulkan_create_texture_image(Bitmap *bitmap) {
	Vulkan_Info *info = &vulkan_info;

	bitmap->gpu_info = platform_malloc(sizeof(Vulkan_Texture));
	Vulkan_Texture *texture = (Vulkan_Texture *)bitmap->gpu_info;
	*texture = {};

	if (bitmap->channels == 1)
		texture->image_format = VK_FORMAT_R8_SRGB;

    VkDeviceSize image_size = bitmap->width * bitmap->height * bitmap->channels;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
	
    vulkan_create_buffer(info->device, info->physical_device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

	void *data;
	vkMapMemory(info->device, staging_buffer_memory, 0, image_size, 0, &data);
	memcpy(data, bitmap->memory, image_size);
	vkUnmapMemory(info->device, staging_buffer_memory);

	vulkan_create_image(info->device, info->physical_device, bitmap->width, bitmap->height, texture->image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->image, texture->image_memory);

	vulkan_transition_image_layout(info, texture->image, texture->image_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan_copy_buffer_to_image(info, staging_buffer, texture->image, (u32)bitmap->width, (u32)bitmap->height);
    vulkan_transition_image_layout(info, texture->image, texture->image_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(info->device, staging_buffer, nullptr);
    vkFreeMemory(info->device, staging_buffer_memory, nullptr);
}

internal void
vulkan_create_texture_image_view(Vulkan_Texture *texture) {
	texture->image_view = vulkan_create_image_view(vulkan_info.device, texture->image, texture->image_format, VK_IMAGE_ASPECT_COLOR_BIT);
}

internal void
vulkan_create_texture_sampler(Vulkan_Texture *texture, u32 texture_parameters) {
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(vulkan_info.physical_device, &properties);
	
	VkSamplerCreateInfo sampler_info = {};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	switch(texture_parameters) {
		case TEXTURE_PARAMETERS_DEFAULT:
		sampler_info.minFilter    = VK_FILTER_LINEAR;
		sampler_info.magFilter    = VK_FILTER_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;

		case TEXTURE_PARAMETERS_CHAR:
		sampler_info.minFilter    = VK_FILTER_LINEAR;
		sampler_info.magFilter    = VK_FILTER_NEAREST;
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
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 0.0f;

	if (vkCreateSampler(vulkan_info.device, &sampler_info, nullptr, &texture->sampler) != VK_SUCCESS) {
        logprint("vulkan_create_texture_sampler()", "failed to create texture sampler\n");
    }
}

internal void
vulkan_create_texture(Bitmap *bitmap, u32 texture_parameters) {
	vulkan_create_texture_image(bitmap);
    vulkan_create_texture_image_view((Vulkan_Texture *)bitmap->gpu_info);
    vulkan_create_texture_sampler((Vulkan_Texture *)bitmap->gpu_info, texture_parameters);
}

internal void
vulkan_set_bitmap(Descriptor_Set *set, Bitmap *bitmap, u32 binding) {
	VkDescriptorSet *vulkan_set = (VkDescriptorSet *)set->gpu_info[vulkan_info.current_frame];
	Vulkan_Texture *texture = (Vulkan_Texture *)bitmap->gpu_info;

	VkDescriptorImageInfo image_info = {};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = texture->image_view;
    image_info.sampler = texture->sampler;

    VkWriteDescriptorSet descriptor_writes[1] = {};

    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = *vulkan_set;
    descriptor_writes[0].dstBinding = binding;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].pImageInfo = &image_info;

    vkUpdateDescriptorSets(vulkan_info.device, ARRAY_COUNT(descriptor_writes), descriptor_writes, 0, nullptr);
}

//
// Init, cleanup and recreation
//

internal void
vulkan_create_swap_chain_image_views(Vulkan_Info *info) {
	info->swap_chain_image_views.resize(info->swap_chain_images.get_size());
	for (u32 i = 0; i < info->swap_chain_images.get_size(); i++) {
		info->swap_chain_image_views[i] = vulkan_create_image_view(info->device, info->swap_chain_images[i], info->swap_chain_image_format, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

internal void
vulkan_cleanup_swap_chain(Vulkan_Info *info) {
	for (u32 i = 0; i < info->swap_chain_framebuffers.get_size(); i++) {
		vkDestroyFramebuffer(info->device, info->swap_chain_framebuffers[i], nullptr);
	}

	for (u32 i = 0; i < info->swap_chain_image_views.get_size(); i++) {
		vkDestroyImageView(info->device, info->swap_chain_image_views[i], nullptr);
	}

	vkDestroySwapchainKHR(info->device, info->swap_chains[0], nullptr);
}

internal void
vulkan_recreate_swap_chain(Vulkan_Info *info) {
	//vkDeviceWaitIdle(info->device);

	if (info->window_width == 0 || info->window_height == 0) {
		/*SDL_Event event;
		SDL_WindowEvent *window_event = &event.window;
		info->window_width = window_event->data1;
		info->window_height = window_event->data2;
		SDL_WaitEvent(&event);
		int i = 0;*/
	}

	vkDeviceWaitIdle(info->device);

	vulkan_cleanup_swap_chain(info);

	vkDestroyImageView(info->device, info->depth_image_view, nullptr);
    vkDestroyImage(info->device, info->depth_image, nullptr);
    vkFreeMemory(info->device, info->depth_image_memory, nullptr);

	vulkan_create_swap_chain(info);
	vulkan_create_swap_chain_image_views(info);
	vulkan_create_depth_resources(info);
	vulkan_create_frame_buffers(info);
}

internal void
vulkan_cleanup() {
	Vulkan_Info *info = &vulkan_info;
	vkDeviceWaitIdle(info->device);

	vulkan_cleanup_swap_chain(info);
	
	// Depth buffer
	vkDestroyImageView(info->device, info->depth_image_view, nullptr);
    vkDestroyImage(info->device, info->depth_image, nullptr);
    vkFreeMemory(info->device, info->depth_image_memory, nullptr);

	// Texture Image
	//vkDestroySampler(info->device, info->texture_sampler, nullptr);
	//vkDestroyImageView(info->device, info->texture_image_view, nullptr);
	//vkDestroyImage(info->device, info->texture_image, nullptr);
    //vkFreeMemory(info->device, info->texture_image_memory, nullptr);

	// Uniform buffer
	for (u32 i = 0; i < info->MAX_FRAMES_IN_FLIGHT; i++) {
		//vkDestroyBuffer(info->device, info->uniform_buffers[i], nullptr);
		//vkFreeMemory(info->device, info->uniform_buffers_memory[i], nullptr);
	}

	//vkDestroyDescriptorPool(info->device, info->descriptor_pool, nullptr);
	//vkDestroyDescriptorSetLayout(info->device, info->descriptor_set_layout, nullptr);

	vkDestroyBuffer(info->device, info->static_buffer, nullptr);
	vkFreeMemory(info->device, info->static_buffer_memory, nullptr);

	vkDestroyBuffer(info->device, info->uniform_buffer, nullptr);
	vkFreeMemory(info->device, info->uniform_buffer_memory, nullptr);
	
	//vkDestroyPipeline(info->device, info->graphics_pipeline, nullptr);
	//vkDestroyPipelineLayout(info->device, info->pipeline_layout, nullptr);
	
	vkDestroyRenderPass(info->device, info->render_pass, nullptr);

	for (u32 i = 0; i < info->MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(info->device, info->image_available_semaphore[i], nullptr);
	    vkDestroySemaphore(info->device, info->render_finished_semaphore[i], nullptr);
	    vkDestroyFence(info->device, info->in_flight_fence[i], nullptr);
	}
	
	vkDestroyCommandPool(info->device, info->command_pool, nullptr);

	vkDestroyDevice(info->device, nullptr);

	vkDestroySurfaceKHR(info->instance, info->surface, nullptr);
	
	if (info->validation_layers.enable)
		vulkan_destroy_debug_utils_messenger_ext(info->instance, info->debug_messenger, nullptr);

	vkDestroyInstance(info->instance, nullptr);
}

internal void
vulkan_init_presentation_settings(Vulkan_Info *info) {
	// Start frame
	info->begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info->begin_info.flags = 0;				   // Optional
	info->begin_info.pInheritanceInfo = nullptr; // Optional
/*
	info->clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
*/
	info->clear_values[1].depthStencil = {1.0f, 0};

	info->render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info->render_pass_info.renderPass = info->render_pass;
	info->render_pass_info.framebuffer = info->swap_chain_framebuffers[info->image_index];
	info->render_pass_info.renderArea.offset = {0, 0};
	info->render_pass_info.renderArea.extent = info->swap_chain_extent;
	info->render_pass_info.clearValueCount = ARRAY_COUNT(info->clear_values);
	info->render_pass_info.pClearValues = info->clear_values;
/*
	info->scissor.offset = {0, 0};
	info->scissor.extent = info->swap_chain_extent;

	info->viewport.x = 0.0f;
	info->viewport.y = 0.0f;
	info->viewport.width = static_cast<float>(info->swap_chain_extent.width);
	info->viewport.height = static_cast<float>(info->swap_chain_extent.height);
	info->viewport.minDepth = 0.0f;
	info->viewport.maxDepth = 1.0f;
*/
	// End of frame
	info->submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info->submit_info.waitSemaphoreCount = 1;
	info->submit_info.pWaitSemaphores = &info->image_available_semaphore[info->current_frame];
	info->submit_info.pWaitDstStageMask = info->wait_stages;
	info->submit_info.commandBufferCount = 1;
	info->submit_info.pCommandBuffers = &info->command_buffers[info->current_frame];	
	info->submit_info.signalSemaphoreCount = 1;
	info->submit_info.pSignalSemaphores = &info->render_finished_semaphore[info->current_frame];

	info->present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info->present_info.waitSemaphoreCount = 1;
	info->present_info.pWaitSemaphores = &info->render_finished_semaphore[info->current_frame];
	info->present_info.swapchainCount = ARRAY_COUNT(info->swap_chains);
	info->present_info.pSwapchains = info->swap_chains;
	info->present_info.pImageIndices = &info->image_index;
	info->present_info.pResults = nullptr; // Optional
}

void vulkan_sdl_init(SDL_Window *sdl_window) {
	Vulkan_Info *info = &vulkan_info;

	SDL_GetWindowSize(sdl_window, &info->window_width, &info->window_height);

	if (info->validation_layers.enable && !vulkan_check_validation_layer_support(info->validation_layers)) {
		logprint("vulkan_init()", "validation layers requested, but not avaiable\n");
	}

	u32 instance_extensions_count;
	if (SDL_Vulkan_GetInstanceExtensions(sdl_window, &instance_extensions_count, NULL) == SDL_FALSE) {
		logprint("main", "nullptr SDL_Vulkan_GetInstanceExtensions failed\n");
	}
	const char **instance_extensions = ARRAY_MALLOC(const char *, instance_extensions_count + 1);
	if (SDL_Vulkan_GetInstanceExtensions(sdl_window, &instance_extensions_count, instance_extensions) == SDL_FALSE) {
		logprint("main", "failed to get instance extensions\n");
	}
	instance_extensions[instance_extensions_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	vulkan_create_instance(info, instance_extensions, instance_extensions_count + 1);
	platform_free(instance_extensions);

	vulkan_setup_debug_messenger(info);

	if (SDL_Vulkan_CreateSurface(sdl_window, info->instance, &info->surface) == SDL_FALSE) {
		logprint("main", "vulkan surface failed being created\n");
	}

	// vulkan stuff
	vulkan_pick_physical_device(info);
	vkGetPhysicalDeviceProperties(info->physical_device, &info->physical_device_properties);
	info->uniform_buffer_min_alignment = info->physical_device_properties.limits.minUniformBufferOffsetAlignment;
	vulkan_create_logical_device(info);
	vulkan_create_swap_chain(info);
	vulkan_create_swap_chain_image_views(info);
	vulkan_create_render_pass(info);
	vulkan_create_sync_objects(info);

	vulkan_create_command_pool(info);
    vulkan_create_command_buffers(info);
	vulkan_create_depth_resources(info);
	vulkan_create_frame_buffers(info);

	vulkan_create_buffer(info->device, 
                         info->physical_device,
                         VULKAN_STATIC_BUFFER_SIZE, 
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         info->static_buffer,
                         info->static_buffer_memory);


	vulkan_create_buffer(info->device, 
                     info->physical_device,
                     1000000, 
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     info->uniform_buffer,
                     info->uniform_buffer_memory);
	
	vkMapMemory(info->device, info->uniform_buffer_memory, 0, VK_WHOLE_SIZE, 0, &vulkan_info.uniform_data);

	vulkan_init_presentation_settings(info);
}

//
// Render
//

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

	vkCmdSetViewport(vulkan_active_cmd_buffer(&vulkan_info), 0, 1, &viewport);
}

void vulkan_set_scissor(u32 width, u32 height) {
	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent = { width, height };

	vkCmdSetScissor(vulkan_active_cmd_buffer(&vulkan_info), 0, 1, &scissor);
}

void vulkan_bind_pipeline(Render_Pipeline *pipeline) {
	vulkan_info.pipeline_layout = pipeline->pipeline_layout; // to use when binding sets later
	vkCmdBindPipeline(vulkan_active_cmd_buffer(&vulkan_info), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->graphics_pipeline);
}

void vulkan_reset_descriptor_sets(Assets *assets) {
	Asset_Array *array = &assets->types[ASSET_TYPE_SHADER];
	for (u32 i = 0; i < array->num_of_assets; i++) {
		Shader *shader = (Shader *)&array->data[i].memory;
		for (u32 layout_i = 0; layout_i < shader->layout_count; layout_i++) {
			shader->vulkan_infos[layout_i].sets_count = 0;
		}
    }
}

void vulkan_start_frame() {
	// Waiting for the previous frame
	vkWaitForFences(vulkan_info.device, 1, &vulkan_info.in_flight_fence[vulkan_info.current_frame], VK_TRUE, UINT64_MAX);
	VkResult result = vkAcquireNextImageKHR(vulkan_info.device,
                                            vulkan_info.swap_chains[0],
                                            UINT64_MAX,
                                            vulkan_info.image_available_semaphore[vulkan_info.current_frame],
                                            VK_NULL_HANDLE,
                                            &vulkan_info.image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
         vulkan_recreate_swap_chain(&vulkan_info);
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		logprint("vulkan_draw_frame()", "failed to acquire swap chain");
	}

	vulkan_info.render_pass_info.framebuffer = vulkan_info.swap_chain_framebuffers[vulkan_info.image_index];

	vkResetFences(vulkan_info.device, 1, &vulkan_info.in_flight_fence[vulkan_info.current_frame]);
	vkResetCommandBuffer(vulkan_active_cmd_buffer(&vulkan_info), 0);
	
	if (vkBeginCommandBuffer(vulkan_active_cmd_buffer(&vulkan_info), &vulkan_info.begin_info) != VK_SUCCESS) {
		logprint("vulkan_record_command_buffer()", "failed to begin recording command buffer\n");
	}	

	vkCmdBeginRenderPass(vulkan_active_cmd_buffer(&vulkan_info), &vulkan_info.render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

void vulkan_end_frame() {
	vkCmdEndRenderPass(vulkan_active_cmd_buffer(&vulkan_info));

	if (vkEndCommandBuffer(vulkan_active_cmd_buffer(&vulkan_info)) != VK_SUCCESS) {
		logprint("vulkan_record_command_buffer()", "failed to record command buffer\n");
	}

	vulkan_info.submit_info.pWaitSemaphores = &vulkan_info.image_available_semaphore[vulkan_info.current_frame];
	vulkan_info.submit_info.pCommandBuffers = &vulkan_info.command_buffers[vulkan_info.current_frame];	
	vulkan_info.submit_info.pSignalSemaphores = &vulkan_info.render_finished_semaphore[vulkan_info.current_frame];

	if (vkQueueSubmit(vulkan_info.graphics_queue, 1, &vulkan_info.submit_info, vulkan_info.in_flight_fence[vulkan_info.current_frame]) != VK_SUCCESS) {
		logprint("vulkan_draw_frame()", "failed to submit draw command buffer\n");
	}

	vulkan_info.present_info.pWaitSemaphores = &vulkan_info.render_finished_semaphore[vulkan_info.current_frame];
	vulkan_info.present_info.pImageIndices = &vulkan_info.image_index;
	
	VkResult result = vkQueuePresentKHR(vulkan_info.present_queue, &vulkan_info.present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkan_info.framebuffer_resized) {
		vulkan_info.framebuffer_resized = false;
		vulkan_recreate_swap_chain(&vulkan_info);
		vulkan_info.render_pass_info.renderArea.extent = vulkan_info.swap_chain_extent;
		return;
	} else if (result != VK_SUCCESS) {
		logprint("vulkan_draw_frame()", "failed to acquire swap chain");
	}

	vulkan_info.current_frame = (vulkan_info.current_frame + 1) % vulkan_info.MAX_FRAMES_IN_FLIGHT;
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

	vulkan_mesh->vertices_offset = vulkan_get_next_offset(&vulkan_info.static_buffer_offset, buffer_size);

    vulkan_update_buffer(&vulkan_info, &vulkan_info.static_buffer, &vulkan_info.static_buffer_memory, memory, buffer_size, vulkan_mesh->vertices_offset);
    vulkan_mesh->indices_offset = vulkan_mesh->vertices_offset + vertices_size;
    
    platform_free(memory);
    mesh->gpu_info = (void*)vulkan_mesh;
}

void vulkan_draw_mesh(Mesh *mesh) {
    Vulkan_Mesh *vulkan_mesh = (Vulkan_Mesh*)mesh->gpu_info;
    VkDeviceSize offsets[] = { vulkan_mesh->vertices_offset };
    vkCmdBindVertexBuffers(vulkan_active_cmd_buffer(&vulkan_info), 0, 1, &vulkan_info.static_buffer, offsets);
    vkCmdBindIndexBuffer(vulkan_active_cmd_buffer(&vulkan_info), vulkan_info.static_buffer, vulkan_mesh->indices_offset, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(vulkan_active_cmd_buffer(&vulkan_info), mesh->indices_count, 1, 0, 0, 0);
}

void vulkan_push_constants(Descriptor_Set *push_constants, void *data) {
	Descriptor *push_constant = &push_constants->descriptors[0];
	vkCmdPushConstants(vulkan_active_cmd_buffer(&vulkan_info), vulkan_info.pipeline_layout, vulkan_convert_shader_stage(push_constant->stages[0]), 0, push_constant->size, data);
}



