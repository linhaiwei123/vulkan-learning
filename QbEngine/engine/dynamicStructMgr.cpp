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
	// cache key
	for (auto&[key, size, count] : view) {
		this->_keys.push_back(key);
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

size_t qb::DynamicStruct::getOffset(std::string key, size_t index) {
	std::pair<std::string, size_t> pair = { key, index };
	auto it = this->addrTable.find(pair);
	if (it == this->addrTable.end())
		assert(0);
	return it->second;
}

const std::vector<std::string>* qb::DynamicStruct::getKeys() {
	assert(this->view.size() != 0);
	return &this->_keys;
}

qb::DynamicData * qb::DynamicStruct::getDynamicData(){
	assert(this->view.size() != 0);
	assert(this->addrTable.size() != 0);
	assert(this->size > 0);

	qb::DynamicData* instance = new DynamicData();
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

void qb::DynamicData::init(DynamicStruct * dynamicStruct){
	this->dynamicStruct = dynamicStruct;
	this->count = -1;
}

void qb::DynamicData::build() {
	assert(this->count > 0);
	this->_data = (char*)malloc(this->dynamicStruct->size * this->count);
}

size_t qb::DynamicData::getTotalSize(){
	assert(this->count > 0);
	return this->dynamicStruct->size * this->count;
}

size_t qb::DynamicData::getUnitSize(){
	return this->dynamicStruct->size;
}

size_t qb::DynamicData::getOffset(std::string key, size_t index){
	return this->dynamicStruct->getOffset(key, index);
}

void qb::DynamicData::setData(size_t countIdx, std::string key, size_t index, void * src) {
	assert(countIdx < count);
	void* dist = (char*)this->_data + this->getOffset(key, index) + countIdx * this->getUnitSize();
	size_t size = this->dynamicStruct->getKeySize(key);
	memcpy(dist, src, size);
}

void qb::DynamicData::setData(std::string key, void * src){
	this->setData(0, key, 0, src);
}

void qb::DynamicData::setData(std::string key, size_t index, void * src){
	this->setData(0, key, index, src);
}

void qb::DynamicData::setData(size_t countIdx, std::string key, void * src){
	this->setData(countIdx, key, 0, src);
}

void qb::DynamicData::setVertexData(std::vector<std::vector<std::any>> src){
	const std::vector<std::string>* keys = this->dynamicStruct->getKeys();
	for (size_t countIdx = 0; countIdx < src.size(); countIdx++) {
		auto& vertex = src[countIdx];
		for (size_t i = 0; i < vertex.size(); i++) {
			auto& key = (*keys)[i];
			auto& attr = vertex[i];
			this->setData(countIdx, key, &attr);
		}
	}
}

void* qb::DynamicData::getMappingAddr(){
	return this->_data;
}

void qb::DynamicData::destroy() {
	free(this->_data);
}
