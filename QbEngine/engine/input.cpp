#include "input.h"
#include "app.h"
void qb::InputMgr::keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods){
	auto win = reinterpret_cast<qb::Win*>(glfwGetWindowUserPointer(window));
	auto inputMgr = win->app->inputMgr;
	for(size_t i = 0; i < inputMgr.keyCallbackRegisters.size(); i++)
		inputMgr.keyCallbackRegisters[i](key, scancode, action, mods);
}
void qb::InputMgr::characterCallback(GLFWwindow * window, unsigned int codepoint){
	auto win = reinterpret_cast<qb::Win*>(glfwGetWindowUserPointer(window));
	auto inputMgr = win->app->inputMgr;
	for (size_t i = 0; i < inputMgr.characterRegisters.size(); i++)
		inputMgr.characterRegisters[i](codepoint);
}
void qb::InputMgr::mouseButtonCallback(GLFWwindow * window, int button, int action, int mods){
	auto win = reinterpret_cast<qb::Win*>(glfwGetWindowUserPointer(window));
	auto inputMgr = win->app->inputMgr;
	for (size_t i = 0; i < inputMgr.mouseBtnRegisters.size(); i++)
		inputMgr.mouseBtnRegisters[i](button, action, mods);
}
void qb::InputMgr::cursorPositionCallback(GLFWwindow * window, double xpos, double ypos){
	auto win = reinterpret_cast<qb::Win*>(glfwGetWindowUserPointer(window));
	auto inputMgr = win->app->inputMgr;
	for (size_t i = 0; i < inputMgr.mouseMoveRegisters.size(); i++)
		inputMgr.mouseMoveRegisters[i](xpos, ypos);
}

 void qb::InputMgr::scrollCallback(GLFWwindow * window, double xoffset, double yoffset){
	 auto win = reinterpret_cast<qb::Win*>(glfwGetWindowUserPointer(window));
	 auto inputMgr = win->app->inputMgr;
	for (size_t i = 0; i < inputMgr.scrollRegisters.size(); i++)
		inputMgr.scrollRegisters[i](xoffset, yoffset);
}

void qb::InputMgr::init(App * app){
	this->app = app;
	// input callback
	glfwSetKeyCallback(app->win.window, InputMgr::keyCallback);
	glfwSetCharCallback(app->win.window, InputMgr::characterCallback);
	glfwSetCursorPosCallback(app->win.window, InputMgr::cursorPositionCallback);
	glfwSetMouseButtonCallback(app->win.window, InputMgr::mouseButtonCallback);
	glfwSetScrollCallback(app->win.window, InputMgr::scrollCallback);
}

void qb::InputMgr::onKey(std::function<void(int key, int scancode, int action, int mods)> func){
	for (size_t i = 0; i < keyCallbackRegisters.size(); i++) {
		if (&keyCallbackRegisters[i] == &func) {
			log_error("key callback register already existed!");
			assert(0);
		}
	}
	keyCallbackRegisters.push_back(func);
}

void qb::InputMgr::onChar(std::function<void(unsigned int codepoint)> func){
	for (size_t i = 0; i < characterRegisters.size(); i++) {
		if (&characterRegisters[i] == &func) {
			log_error("char callback register already existed!");
			assert(0);
		}
	}
	characterRegisters.push_back(func);
}

void qb::InputMgr::onMouseBtn(std::function<void(int button, int action, int mods)> func){
	for (size_t i = 0; i < mouseBtnRegisters.size(); i++) {
		if (&mouseBtnRegisters[i] == &func) {
			log_error("mouse btn callback register already existed!");
			assert(0);
		}
	}
	mouseBtnRegisters.push_back(func);
}

void qb::InputMgr::onMouseMove(std::function<void(double xpos, double ypos)> func){
	for (size_t i = 0; i < mouseMoveRegisters.size(); i++) {
		if (&mouseMoveRegisters[i] == &func) {
			log_error("mouse move callback register already existed!");
			assert(0);
		}
	}
	mouseMoveRegisters.push_back(func);
}

void qb::InputMgr::onScroll(std::function<void(double xoffset, double yoffset)> func){
	for (size_t i = 0; i < scrollRegisters.size(); i++) {
		if (&scrollRegisters[i] == &func) {
			log_error("scroll callback register already existed!");
			assert(0);
		}
	}
	scrollRegisters.push_back(func);
}

void qb::InputMgr::offKey(std::function<void(int key, int scancode, int action, int mods)> func){
	size_t idx = -1;
	for (size_t i = 0; i < keyCallbackRegisters.size(); i++) {
		if (&keyCallbackRegisters[i] == &func) {
			idx = i;
			break;
		}
	}
	if(idx >= 0)
		keyCallbackRegisters.erase(keyCallbackRegisters.begin() + idx);
}

void qb::InputMgr::offChar(std::function<void(unsigned int codepoint)> func){
	size_t idx = -1;
	for (size_t i = 0; i < characterRegisters.size(); i++) {
		if (&characterRegisters[i] == &func) {
			idx = i;
			break;
		}
	}
	if (idx >= 0)
		characterRegisters.erase(characterRegisters.begin() + idx);
}

void qb::InputMgr::offMouseBtn(std::function<void(int button, int action, int mods)> func){
	size_t idx = -1;
	for (size_t i = 0; i < mouseBtnRegisters.size(); i++) {
		if (&mouseBtnRegisters[i] == &func) {
			idx = i;
			break;
		}
	}
	if (idx >= 0)
		mouseBtnRegisters.erase(mouseBtnRegisters.begin() + idx);
}

void qb::InputMgr::offMouseMove(std::function<void(double xpos, double ypos)> func){
	size_t idx = -1;
	for (size_t i = 0; i < mouseMoveRegisters.size(); i++) {
		if (&mouseMoveRegisters[i] == &func) {
			idx = i;
			break;
		}
	}
	if (idx >= 0)
		mouseMoveRegisters.erase(mouseMoveRegisters.begin() + idx);
}

void qb::InputMgr::offScroll(std::function<void(double xoffset, double yoffset)> func){
	size_t idx = -1;
	for (size_t i = 0; i < scrollRegisters.size(); i++) {
		if (&scrollRegisters[i] == &func) {
			idx = i;
			break;
		}
	}
	if (idx >= 0)
		scrollRegisters.erase(scrollRegisters.begin() + idx);
}

glm::dvec2 qb::InputMgr::getMousePos(){
	glm::dvec2 pos;
	glfwGetCursorPos(app->win.window, &pos.x, &pos.y);
	return pos;
}

void qb::InputMgr::destroy(){

}
