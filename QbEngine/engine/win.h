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
	class Win {
	public:
		const int WIDTH = 800;
		const int HEIGHT = 600;
		bool resized = false;
		GLFWwindow* window;
		App *app;
	public:
		Win() = default;

		void init(App *app);

		static void resizeCallback(GLFWwindow* window, int width, int height) {
			auto win = reinterpret_cast<Win*>(glfwGetWindowUserPointer(window));
			win->resized = true;
		}

		void mainLoop(std::function<void()> callback);

		void destroy();
	};
};