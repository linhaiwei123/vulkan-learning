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
#include <set>
namespace qb {
	class App;
	class Device {
	public:
		VkPhysicalDevice physical = VK_NULL_HANDLE;
		VkDevice logical = VK_NULL_HANDLE;
		App *app;
		struct QueueFamilyIndices {
			std::optional<uint32_t> graphics;
			std::optional<uint32_t> compute;
			std::optional<uint32_t> transfer;
			std::optional<uint32_t> present;
			bool isFound() {
				return graphics.has_value() && 
					    compute.has_value() && 
					   transfer.has_value() &&
					    present.has_value();
			}
		} queueFamilyIndices;
		std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		struct SwapChainSupport {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		} swapchainsupport;
		struct Queue {
			VkQueue graphics;
			VkQueue present;
		} queues;
	private:
		uint32_t _findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	public:
		Device() = default;

		void init(App *app);

		void destroy();

		VkDeviceMemory&& allocBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties);
		VkDeviceMemory&& allocImageMemory(VkImage image, VkMemoryPropertyFlags properties);
	};
};