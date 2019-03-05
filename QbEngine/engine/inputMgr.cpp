#include "inputMgr.h"
#include "app.h"
void qb::InputMgr::keyCallback(GLFWwindow * window, int key, int scanCode, int action, int mods){
	auto win = reinterpret_cast<qb::Win*>(glfwGetWindowUserPointer(window));
	auto inputMgr = win->app->inputMgr;
	inputMgr.eventEmitter->emit("key", { EventDetail::Key{key, scanCode, action, mods} });
	
}
void qb::InputMgr::characterCallback(GLFWwindow * window, unsigned int codePoint){
	auto win = reinterpret_cast<qb::Win*>(glfwGetWindowUserPointer(window));
	auto inputMgr = win->app->inputMgr;
	inputMgr.eventEmitter->emit("char", { EventDetail::Char{codePoint} });
}
void qb::InputMgr::mouseButtonCallback(GLFWwindow * window, int button, int action, int mods){
	auto win = reinterpret_cast<qb::Win*>(glfwGetWindowUserPointer(window));
	auto inputMgr = win->app->inputMgr;
	inputMgr.eventEmitter->emit("mouse-button", { EventDetail::MouseBtn{button, action, mods} });
}
void qb::InputMgr::cursorPositionCallback(GLFWwindow * window, double x, double y){
	auto win = reinterpret_cast<qb::Win*>(glfwGetWindowUserPointer(window));
	auto inputMgr = win->app->inputMgr;
	inputMgr.eventEmitter->emit("mouse-move", { EventDetail::MouseMove{x, y} });
}

 void qb::InputMgr::scrollCallback(GLFWwindow * window, double x, double y){
	 auto win = reinterpret_cast<qb::Win*>(glfwGetWindowUserPointer(window));
	 auto inputMgr = win->app->inputMgr;
	 inputMgr.eventEmitter->emit("mouse-scroll", { EventDetail::MouseScroll{x, y} });
}

void qb::InputMgr::init(App * app){
	this->app = app;
	this->eventEmitter = app->eventMgr.getEventEmitter("inputMgr");
	// input callback
	glfwSetKeyCallback(app->win.window, InputMgr::keyCallback);
	glfwSetCharCallback(app->win.window, InputMgr::characterCallback);
	glfwSetCursorPosCallback(app->win.window, InputMgr::cursorPositionCallback);
	glfwSetMouseButtonCallback(app->win.window, InputMgr::mouseButtonCallback);
	glfwSetScrollCallback(app->win.window, InputMgr::scrollCallback);
}

glm::dvec2 qb::InputMgr::getMousePos(){
	glm::dvec2 pos;
	glfwGetCursorPos(app->win.window, &pos.x, &pos.y);
	return pos;
}

void qb::InputMgr::destroy(){
	this->eventEmitter = nullptr;
}
