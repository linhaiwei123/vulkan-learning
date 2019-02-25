#include "surface.h"
#include "app.h"
void qb::Surf::init(App* app) {
	this->app = app;
	vk_check(glfwCreateWindowSurface(app->inst.instance, app->win.window, nullptr, &surface));
}

void qb::Surf::destroy() {
	vkDestroySurfaceKHR(app->inst.instance, surface, nullptr);
}