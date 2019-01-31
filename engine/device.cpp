#include "device.h"
#include "app.h"
void qb::Device::init(App* app) {
	this->app = app;
	//physical device
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(app->inst.instance, &physicalDeviceCount, nullptr);
	assert(physicalDeviceCount != 0);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(app->inst.instance, &physicalDeviceCount, physicalDevices.data());
	
	for (const auto& physicalDevice : physicalDevices) {
		//queue family indices
		QueueFamilyIndices indices;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueCount <= 0) continue;
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphics = i;
			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
				indices.compute = i;
			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
				indices.transfer = i;
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, app->surf.surface, &presentSupport);
			if (presentSupport)
				indices.present = i;
			if (indices.isFound()) break;
			i++;
		}

		// device extension
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		assert(extensionCount != 0);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
		std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		bool extensionFound = requiredExtensions.empty();

		// device swapchain support
		SwapChainSupport swapchainsupport;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, app->surf.surface, &swapchainsupport.capabilities);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, app->surf.surface, &formatCount, nullptr);
		assert(formatCount != 0);
		swapchainsupport.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, app->surf.surface, &formatCount, swapchainsupport.formats.data());
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, app->surf.surface, &presentModeCount, nullptr);
		assert(presentModeCount != 0);
		swapchainsupport.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, app->surf.surface, &presentModeCount, swapchainsupport.presentModes.data());
		bool swapchainFound = !swapchainsupport.formats.empty() && !swapchainsupport.presentModes.empty();

		if (indices.isFound() && extensionFound && swapchainFound) {
			this->physical = physicalDevice;
			this->queueFamilyIndices = indices;
			this->swapchainsupport = swapchainsupport;
			break;
		}
	}
	assert(this->physical != VK_NULL_HANDLE);

	//warning! only one queue server for three index
	//device queue
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = this->queueFamilyIndices.graphics.value();
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	//logical device
	{
		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledLayerCount = static_cast<uint32_t>(app->inst.validationLayers.size());
		createInfo.ppEnabledLayerNames = app->inst.validationLayers.data();
		vk_check(vkCreateDevice(physical, &createInfo, nullptr, &logical));
	}
	// graphics queue
	vkGetDeviceQueue(logical, queueFamilyIndices.graphics.value(), 0, &queues.graphics);
	// present queue
	vkGetDeviceQueue(logical, queueFamilyIndices.present.value(), 0, &queues.present);

	//depth format
	for (VkFormat depthFormat : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical, depthFormat, &props);
		if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			this->depthFormat = depthFormat;
		}
	}
}

void qb::Device::destroy() {
	vkDestroyDevice(logical, nullptr);
}

VkDeviceMemory&& qb::Device::allocBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties) {
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(logical, buffer, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = _findMemoryType(memRequirements.memoryTypeBits, properties);
	VkDeviceMemory mem;
	vk_check(vkAllocateMemory(logical, &allocInfo, nullptr, &mem));
	vkBindBufferMemory(logical, buffer, mem, 0);
	return std::forward<VkDeviceMemory>(mem);
}

VkDeviceMemory&& qb::Device::allocImageMemory(VkImage image, VkMemoryPropertyFlags properties){
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(logical, image, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = _findMemoryType(memRequirements.memoryTypeBits, properties);
	VkDeviceMemory mem;
	vk_check(vkAllocateMemory(logical, &allocInfo, nullptr, &mem));
	vkBindImageMemory(logical, image, mem, 0);
	return std::forward<VkDeviceMemory>(mem);
}

uint32_t qb::Device::_findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(app->device.physical, &memProperties);
	uint32_t i;
	for (i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	assert(i != memProperties.memoryTypeCount);
	return -1;
}
