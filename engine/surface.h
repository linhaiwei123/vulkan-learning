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
	class Surf {
	public:
		App *app;
		VkSurfaceKHR surface;
	public:
		Surf() = default;

		void init(App *app);

		void destroy();
	};
};