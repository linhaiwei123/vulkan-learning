#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace qb {
	class App;
	class InputMgr {
	private:
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void characterCallback(GLFWwindow* window, unsigned int codepoint);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	public:
		App *app;
		std::vector<std::function<void(int key, int scancode, int action, int mods)>> keyCallbackRegisters{};
		std::vector<std::function<void(unsigned int codepoint)>> characterRegisters{};
		std::vector<std::function<void(int button, int action, int mods)>> mouseBtnRegisters{};
		std::vector<std::function<void(double xpos, double ypos)>> mouseMoveRegisters{};
		std::vector<std::function<void(double xoffset, double yoffset)>> scrollRegisters{};
	public:
		InputMgr() = default;

		void init(App *app);

		void onKey(std::function<void(int key, int scancode, int action, int mods)> func);
		void onChar(std::function<void(unsigned int codepoint)> func);
		void onMouseBtn(std::function<void(int button, int action, int mods)> func);
		void onMouseMove(std::function<void(double xpos, double ypos)> func);
		void onScroll(std::function<void(double xoffset, double yoffset)> func);

		void offKey(std::function<void(int key, int scancode, int action, int mods)> func);
		void offChar(std::function<void(unsigned int codepoint)> func);
		void offMouseBtn(std::function<void(int button, int action, int mods)> func);
		void offMouseMove(std::function<void(double xpos, double ypos)> func);
		void offScroll(std::function<void(double xoffset, double yoffset)> func);

		glm::dvec2 getMousePos();

		void destroy();
	};
};