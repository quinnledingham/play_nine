//
// Vulkan Debug
//

bool8 vulkan_check_validation_layer_support(Vulkan_Validation_Layers validation_layers) {
  u32 layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  //VkLayerProperties *available_layers = ARRAY_MALLOC(VkLayerProperties, layer_count);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  bool8 all_layers_found = true;
  for (u32 validation_index = 0; validation_index < validation_layers.count; validation_index++) {
    bool8 layer_found = false;

    for (u32 available_index = 0; available_index < layer_count; available_index++) {
      if (strcmp(validation_layers.data[validation_index], available_layers[available_index].layerName)) {
        layer_found = true;
        break;
      }
    }

    if (!layer_found) {
      all_layers_found = false;
      break;
    }
  }

  return all_layers_found;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
	printf("(vulkan) %s\n", callback_data->pMessage);
	return VK_FALSE;
}

void vulkan_populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info) {
  create_info = {};
  create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = vulkan_debug_callback;
}

VkResult vulkan_create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger) {
  PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, create_info, allocator, debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void vulkan_destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator) {
  PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debug_messenger, allocator);
  }
}

void vulkan_setup_debug_messenger() {
	if (!vulkan_context.validation_layers.enable)
		return;

	VkDebugUtilsMessengerCreateInfoEXT create_info;
  vulkan_populate_debug_messenger_create_info(create_info);

	if (vulkan_create_debug_utils_messenger_ext(vulkan_context.instance, &create_info, nullptr, &vulkan_context.debug_messenger) != VK_SUCCESS) {
		printf("(vulkan) vulkan_create_debug_messenger(): failed to set up debug messenger\n");
	}
}

//
// Vulkan Setup
//

bool8 vulkan_create_instance(const char **instance_extensions, u32 instance_extensions_count) {
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
	if (vulkan_context.validation_layers.enable) {
    create_info.enabledLayerCount = vulkan_context.validation_layers.count;
    create_info.ppEnabledLayerNames = vulkan_context.validation_layers.data;

		vulkan_populate_debug_messenger_create_info(debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	} else {
		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;
	}

	if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
    printf("(vulkan) vulkan_create_instance(): failed to create vulkan instance\n");
    return true;
	}

  vulkan_context.instance = instance;

  return false;
}

Vulkan_Version vulkan_get_api_version(u32 api_version) {
	Vulkan_Version version = {};
	version.variant = VK_API_VERSION_VARIANT(api_version);
	version.major   = VK_API_VERSION_MAJOR(api_version);
	version.minor   = VK_API_VERSION_MINOR(api_version);
	version.patch   = VK_API_VERSION_PATCH(api_version);
	return version;
}

Vulkan_Version vulkan_get_nvidia_driver_version(u32 api_version) {
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
		printf("%s: %d.%d.%d.%d\n", name, version.variant, version.major, version.minor, version.patch);
	else
		printf("%s: %d.%d.%d\n", name, version.major, version.minor, version.patch);
}

Vulkan_Swap_Chain_Support_Details vulkan_query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
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

bool8 vulkan_check_device_extension_support(VkPhysicalDevice device, const char **device_extensions, u32 device_extensions_count) {
	u32 available_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &available_count, nullptr);

  VkExtensionProperties *available_extensions = ARRAY_MALLOC(VkExtensionProperties, available_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &available_count, available_extensions);

  bool8 all_extensions_found = true;
  for (u32 required_index = 0; required_index < device_extensions_count; required_index++) {
    bool8 extension_found = false;
    for (u32 available_index = 0; available_index < available_count; available_index++) {
      if (strcmp(available_extensions[available_index].extensionName, device_extensions[required_index])) {
        extension_found = true;
        break;
      }
    }

    if (!extension_found) {
      all_extensions_found = false;
      break;
    }
  }
	
	free(available_extensions);

  return all_extensions_found;
}

Vulkan_Queue_Family_Indices vulkan_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
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

	free(queue_families);

	return indices;
}

bool8 vulkan_is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface, const char **device_extensions, u32 device_extensions_count) {
	bool8 extensions_supported = vulkan_check_device_extension_support(device, device_extensions, device_extensions_count);
	bool8 swap_chain_adequate = false;
	if (extensions_supported) {
		Vulkan_Swap_Chain_Support_Details swap_chain_support = vulkan_query_swap_chain_support(device, surface);
		swap_chain_adequate = swap_chain_support.formats_count && swap_chain_support.present_modes_count;
		free(swap_chain_support.formats);
		free(swap_chain_support.present_modes);
	}

	Vulkan_Queue_Family_Indices indices = vulkan_find_queue_families(device, surface);
	VkPhysicalDeviceProperties device_properties = {};
	VkPhysicalDeviceFeatures device_features = {};
	vkGetPhysicalDeviceProperties(device, &device_properties);
	vkGetPhysicalDeviceFeatures(device, &device_features);

	printf("vulkan_is_device_suitable(): Vulkan Physical Device Limits:\nMax Descriptor Set Samplers: %u\nMax Descriptor Set Uniform Buffers: %u\nMax Uniform Buffer Range: %u\n", device_properties.limits.maxDescriptorSetSamplers, device_properties.limits.maxDescriptorSetUniformBuffers, device_properties.limits.maxUniformBufferRange); 
	return indices.graphics_and_compute_family.found && extensions_supported && swap_chain_adequate && device_features.samplerAnisotropy && device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}


bool8 vulkan_pick_physical_device() {
  vulkan_context.physical_device = VK_NULL_HANDLE;

  u32 device_count = 0;
  vkEnumeratePhysicalDevices(vulkan_context.instance, &device_count, nullptr);
  if (device_count == 0) {
    printf("vulkan_pick_physical_device(): failed to find GPUs with Vulkan support\n");
    return 1;
  }

  printf("vulkan_pick_physical_device(): Number of physical devices = %d\n", device_count);

  VkPhysicalDevice *devices = ARRAY_MALLOC(VkPhysicalDevice, device_count);
  vkEnumeratePhysicalDevices(vulkan_context.instance, &device_count, devices);

  // print all available devies
	printf("vulkan_pick_physical_device(): Available Physical Devices:\n");
	for (u32 device_index = 0; device_index < device_count; device_index++) {
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(devices[device_index], &device_properties);
		
		Vulkan_Version api_version = vulkan_get_api_version(device_properties.apiVersion);
		Vulkan_Version driver_version = vulkan_get_nvidia_driver_version(device_properties.driverVersion);
		
		printf("Vulkan Physical Device %d:\n", device_index);
		vulkan_print_version("  API Version", api_version);
		vulkan_print_version("  Driver Version", driver_version);
		printf("  Device Name %s\n", device_properties.deviceName);
	}

  // pick suitable device
	for (u32 device_index = 0; device_index < device_count; device_index++) {
		if (vulkan_is_device_suitable(devices[device_index], vulkan_context.surface, vulkan_context.device_extensions, ARRAY_COUNT(vulkan_context.device_extensions))) {
			vulkan_context.physical_device = devices[device_index];
			printf("(vulkan) Picking device %d\n", device_index);
			break;
		}
	}

  free(devices);

  if (vulkan_context.physical_device == VK_NULL_HANDLE) {
		printf("vulkan_pick_physical_device(): failed to find a suitable GPU\n");
		return 1;
	}

	vkGetPhysicalDeviceProperties(vulkan_context.physical_device, &vulkan_context.physical_device_properties);
	vulkan_context.queue_family_indices = vulkan_find_queue_families(vulkan_context.physical_device, vulkan_context.surface);

  return 0;
}

s32 vulkan_sdl_init(SDL_Window *sdl_window) {
  printf("(vulkan) initializing...\n");

  if (vulkan_context.validation_layers.enable && !vulkan_check_validation_layer_support(vulkan_context.validation_layers)) {
		printf("(vulkan) vulkan_sdl_init(): validation layers requested, but not avaiable\n");
	}

  u32 instance_extensions_count;
	if (SDL_Vulkan_GetInstanceExtensions(sdl_window, &instance_extensions_count, NULL) == SDL_FALSE) {
		printf("(vulkan) vulkan_sdl_init(): SDL_Vulkan_GetInstanceExtensions failed\n");
		return 1;
	}
  
  u32 total_instance_extensions_count = instance_extensions_count + ARRAY_COUNT(vulkan_context.extra_instance_extensions);
  const char **instance_extensions = ARRAY_MALLOC(const char *, total_instance_extensions_count);

	if (SDL_Vulkan_GetInstanceExtensions(sdl_window, &instance_extensions_count, instance_extensions) == SDL_FALSE) {
		printf("(vulkan) vulkan_sdl_init(): failed to get instance extensions\n");
		return 1;
	}

  // adding extra instance to the end of instance extensions
	instance_extensions[instance_extensions_count] = vulkan_context.extra_instance_extensions[0];

  if (vulkan_create_instance(instance_extensions, total_instance_extensions_count))
    return 1;
  free(instance_extensions);

	vulkan_setup_debug_messenger();

	if (SDL_Vulkan_CreateSurface(sdl_window, vulkan_context.instance, &vulkan_context.surface) == SDL_FALSE) {
		printf("(vulkan) vulkan surface failed being created\n");
		return 1;
	}

  if (vulkan_pick_physical_device()) 
    return 1;

  return 0;
}