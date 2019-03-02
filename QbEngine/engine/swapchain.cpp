#include "swapchain.h"
#include "app.h"
VkSurfaceFormatKHR qb::Swapchain::_chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR qb::Swapchain::_chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	/*for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
		}
	}*/

	return bestMode;
}

VkExtent2D qb::Swapchain::_chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = {
			std::clamp<uint32_t>(app->win.WIDTH,capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp<uint32_t>(app->win.HEIGHT,capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};
		return actualExtent;
	}
}

uint32_t qb::Swapchain::_getImageCount(const VkSurfaceCapabilitiesKHR & capabilities) {
	/*uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount < capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}
	return imageCount;*/
	return capabilities.minImageCount;
}

void qb::Swapchain::init(App* app) {
	this->app = app;

	// create swap chain
	Device::SwapChainSupport& swapchainsupport = app->device.swapchainsupport;
	VkSurfaceFormatKHR surfaceFormat = _chooseSwapSurfaceFormat(swapchainsupport.formats);
	this->format = surfaceFormat.format;
	VkPresentModeKHR presentMode = _chooseSwapPresentMode(swapchainsupport.presentModes);
	VkExtent2D extent = _chooseSwapExtent(swapchainsupport.capabilities);
	this->extent = extent;
	uint32_t imageCount = _getImageCount(swapchainsupport.capabilities);

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = app->surf.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	
	Device::QueueFamilyIndices& indices = app->device.queueFamilyIndices;
	uint32_t queueFamilyIndices[] = {indices.graphics.value(), indices.present.value()};
	if (indices.graphics.value() != indices.present.value()) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = swapchainsupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	vk_check(vkCreateSwapchainKHR(app->device.logical, &createInfo, nullptr, &swapchain));

	// swap chain image
	uint32_t swapchainImageCount;
	vkGetSwapchainImagesKHR(app->device.logical, swapchain, &swapchainImageCount, nullptr);
	assert(swapchainImageCount != 0);
	images.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(app->device.logical, swapchain, &swapchainImageCount, images.data());

	// swap chain image view
	views.resize(images.size());
	for (size_t i = 0; i < images.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY, // r
			VK_COMPONENT_SWIZZLE_IDENTITY, // g
			VK_COMPONENT_SWIZZLE_IDENTITY, // b
			VK_COMPONENT_SWIZZLE_IDENTITY  // a
		};
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0; // mipmap
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0; // texture array
		createInfo.subresourceRange.layerCount = 1;

		vk_check(vkCreateImageView(app->device.logical, &createInfo, nullptr, &views[i]));
	}
}

void qb::Swapchain::destroy() {
	for (auto view : views) {
		vkDestroyImageView(app->device.logical, view, nullptr);
	}
	vkDestroySwapchainKHR(app->device.logical, swapchain, nullptr);
}