#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
#include <optional>
namespace qb {
	class App;
	class Swapchain {
	public:
		App *app;
		VkSwapchainKHR swapchain;
		std::vector<VkImage> images;
		std::vector<VkImageView> views;
		VkFormat format;
		VkExtent2D extent;
	private:
		VkSurfaceFormatKHR _chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR _chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		VkExtent2D _chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		uint32_t _getImageCount(const VkSurfaceCapabilitiesKHR& capabilities);
	public:
		Swapchain() = default;

		void init(App *app);

		void destroy();
	};
};