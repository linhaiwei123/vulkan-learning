#include "win.h"
#include "app.h"
void qb::Win::init(App *app) {
	this->app = app;
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "qb", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, resizeCallback);
}

void qb::Win::mainLoop(std::function<void()> callback) {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		callback();
	}
	vkDeviceWaitIdle(app->device.logical);  
}

void qb::Win::destroy() {
	glfwDestroyWindow(window);
	glfwTerminate();
}