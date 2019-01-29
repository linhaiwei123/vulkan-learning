#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
namespace qb {
	class App;
	class Sync {
	public:
		App *app;
		const int MAX_FRAMES = 2;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		size_t currentFrame = 0;
		uint32_t currentImage = 0;
	public:
		Sync() = default;

		void init(App *app);

		void destroy();
	};
};