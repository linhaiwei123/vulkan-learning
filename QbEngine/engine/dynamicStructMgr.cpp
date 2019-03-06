#include "dynamicStructMgr.h"
#include "app.h"
void qb::DynamicStructMgr::init(App * app){
	this->app = app;
}

qb::DynamicStruct * qb::DynamicStructMgr::getDynamicStruct(std::string name){
	// read dynamicStruct from cache
	auto it = _dynamicStructMap.find(name);
	if (it != _dynamicStructMap.end())
		return it->second;
	DynamicStruct* dynamicStruct = new DynamicStruct();
	dynamicStruct->init(app, name);
	_dynamicStructMap.insert({ name, dynamicStruct });
	return _dynamicStructMap.at(name);
}

void qb::DynamicStructMgr::destroy(){
	for (auto& it : _dynamicStructMap) {
		it.second->destroy();
	}
	for (auto& it : _dynamicStructMap) {
		delete it.second;
	}
}

void qb::DynamicStruct::init(App * app, std::string name){
	this->app = app;
	this->name = name;
}

void qb::DynamicStruct::build(){
	assert(this->view.size() != 0);
	// build table
	this->size = 0;
	for (auto& [key, size, count] : view) {
		
		for (size_t i = 0; i < count; i++) {
			this->addrTable[{key, i}] = this->size;
			this->size += size;
		}
	}
}

size_t qb::DynamicStruct::getKeySize(std::string key){
	assert(this->view.size() != 0);
	for (auto& it : view) {
		auto& _key = std::get<0>(it);
		if (_key == key) {
			return std::get<1>(it);
		}
	}
	assert(0);
	return -1;
}

qb::DynamicStructInstance * qb::DynamicStruct::create(){
	assert(this->view.size() != 0);
	assert(this->addrTable.size() != 0);
	assert(this->size > 0);

	qb::DynamicStructInstance* instance = new DynamicStructInstance();
	instance->init(this);
	this->_instances.push_back(instance);
	return instance;
}

void qb::DynamicStruct::destroy(){
	for (auto& it : _instances) {
		it->destroy();
	}
	for (auto& it : _instances) {
		delete it;
	}
}

void qb::DynamicStructInstance::init(DynamicStruct * dynamicStruct){
	this->dynamicStruct = dynamicStruct;
	this->data = (char*)malloc(this->dynamicStruct->size);
}

size_t qb::DynamicStructInstance::getSize(){
	return this->dynamicStruct->size;
}

size_t qb::DynamicStructInstance::getOffset(std::string key, size_t index){
	std::pair<std::string, size_t> pair = { key, index };
	auto it = this->dynamicStruct->addrTable.find(pair);
	if (it == this->dynamicStruct->addrTable.end())
		assert(0);
	return it->second;
}

void qb::DynamicStructInstance::setData(std::string key, size_t index, void * src){
	void* dist = (char*)this->data + this->getOffset(key, index);
	size_t size = this->dynamicStruct->getKeySize(key);
	memcpy(dist, src, size);
}

void qb::DynamicStructInstance::destroy() {
	free(this->data);
}
