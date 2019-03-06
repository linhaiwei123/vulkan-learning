#include "timerMgr.h"
#include "app.h"
void qb::TimerMgr::init(App * app){
	this->app = app;
	this->_deltaTime = -1;
	this->_totalTime = -1;
	
}

void qb::TimerMgr::update(){
	static auto startTime = std::chrono::high_resolution_clock::now();
	if (!this->_isInitStartTime) {
		this->_isInitStartTime = true;
		this->_preTime = std::chrono::high_resolution_clock::now();
	}
	auto currentTime = std::chrono::high_resolution_clock::now();
	this->_totalTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	this->_deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _preTime).count();
	_preTime = currentTime;
}

float qb::TimerMgr::getDeltaTime() {
	return this->_deltaTime;
}

float qb::TimerMgr::getTotalTime() {
	return this->_totalTime;
}

void qb::TimerMgr::destroy(){

}
