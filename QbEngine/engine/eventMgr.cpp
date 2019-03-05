#include "eventMgr.h"
#include "app.h"
void qb::EventEmitter::on(std::string eventName, std::function<void(qb::Event& event)> cb){
	auto& table = this->_eventOnTable[eventName];
	for (size_t i = 0; i < table.size(); i++) {
		if (&cb == &table[i]) {
			log_error("%s callback already existed!", eventName.c_str());
			assert(0);
		}
	}
	table.push_back(cb);
}

void qb::EventEmitter::off(std::string eventName, std::function<void(qb::Event& event)> cb){
	size_t index = -1;
	auto& table = this->_eventOnTable[eventName];
	for (size_t i = 0; i < table.size(); i++) {
		if (&cb == &table[i]) {
			index = i;
			break;
		}
	}
	if (index >= 0) {
		table.erase(table.begin() + index);
	}

	index = -1;
	table = this->_eventOnceTable[eventName];
	for (size_t i = 0; i < table.size(); i++) {
		if (&cb == &table[i]) {
			index = i;
			break;
		}
	}
	if (index >= 0) {
		table.erase(table.begin() + index);
	}
}

void qb::EventEmitter::once(std::string eventName, std::function<void(qb::Event& event)> cb){
	auto& table = this->_eventOnceTable[eventName];
	for (size_t i = 0; i < table.size(); i++) {
		if (&cb == &table[i]) {
			log_error("%s callback already existed!", eventName.c_str());
			assert(0);
		}
	}
	table.push_back(cb);
}

void qb::EventEmitter::emit(std::string eventName, qb::Event event){
	// on
	{
		auto& table = this->_eventOnTable[eventName];
		for (size_t i = 0; i < table.size(); i++)
			table[i](event);
	}
	// once
	{
		auto& table = this->_eventOnceTable[eventName];
		for (size_t i = 0; i < table.size(); i++)
			table[i](event);
		table = {};
	}
}

void qb::EventEmitter::clear(){
	this->_eventOnceTable = {};
	this->_eventOnTable = {};
}

void qb::EventEmitter::init(App * app, std::string name){
	this->app = app;
	this->name = name;
	this->_eventOnceTable = {};
	this->_eventOnTable = {};
}

void qb::EventEmitter::destroy(){
	this->clear();
}

void qb::EventMgr::init(App * app){
	this->app = app;
}

qb::EventEmitter * qb::EventMgr::getEventEmitter(std::string name){
	// read event emitter from cache
	auto it = _eventEmitterMap.find(name);
	if (it != _eventEmitterMap.end())
		return it->second;
	EventEmitter* eventEmitter = new EventEmitter();
	eventEmitter->init(app, name);
	_eventEmitterMap.insert({ name, eventEmitter });
	return _eventEmitterMap.at(name);
}

void qb::EventMgr::destroy(){
	for (auto& it : _eventEmitterMap) {
		it.second->destroy();
		delete it.second;
	}
}
