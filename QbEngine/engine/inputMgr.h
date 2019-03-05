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
	class EventEmitter;
	class InputMgr {
	private:
		static void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods);
		static void characterCallback(GLFWwindow* window, unsigned int codePoint);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void cursorPositionCallback(GLFWwindow* window, double x, double y);
		static void scrollCallback(GLFWwindow* window, double x, double y);
	public:
		App *app;
		qb::EventEmitter* eventEmitter = nullptr;
	public:
		struct EventDetail {
			struct Key {
				int key;
				int scanCode;
				int action;
				int mods;
			};
			struct Char {
				unsigned int codePoint;
			};
			struct MouseBtn {
				int button;
				int action;
				int mods;
			};
			struct MouseMove {
				double x;
				double y;
			};
			struct MouseScroll {
				double x;
				double y;
			};
		};
	public:
		InputMgr() = default;

		void init(App *app);

		glm::dvec2 getMousePos();

		void destroy();
		
	};
};