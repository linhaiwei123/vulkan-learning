#include "actor.h"

template<typename T>
T * qb::ecs::Actor::addComponent() {
	T* comp = new T();
	T->init(this);
	this->components.push_back(comp);
	return comp;
}

template<typename T>
T* qb::ecs::Actor::removeComponent() {
	T* result = getComponent<T>();
	components.erase(std::remove(components.begin(), components.end(), result), components.end());
	result->parent = nullptr;
}

template<typename T>
T* qb::ecs::Actor::getComponent() {
	T* result = nullptr;
	for (size_t i = 0; i < components.size(); i++) {
		auto& comp = components[i];
		if (typeid(*comp) == typeid(T)) {
			result = comp;
			break;
		}
	}
	return result;
}

template<typename T>
std::vector<T*> qb::ecs::Actor::removeComponents() {
	std::vector<T*> result = getComponents<T>();
	for (size_t i = 0; i < result.size(); i++) {
		components.erase(std::remove(components.begin(), components.end(), result[i]), components.end());
		result[i]->parent = nullptr;
	}
}

template<typename T>
std::vector<T*> qb::ecs::Actor::getComponents() {
	std::vector<T*> result;
	for (size_t i = 0; i < components.size(); i++) {
		auto& comp = components[i];
		if (typeid(*comp) == typeid(T))
			result.push_back(comp);
	}
	return result;
}

void qb::ecs::Actor::init(){
	this->transform.position = glm::vec3(0.0f);
	this->transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	this->transform.scale = glm::vec3(1.0f);
	this->name = "";
	this->uuid = boost::uuids::random_generator()();
	this->active = true;
}

void qb::ecs::Actor::update(float dt){
	for (size_t i = 0; i < components.size(); i++) {
		auto& comp = components[i];
		if (comp->active) {
			comp->update(dt);
		}
	}
}

void qb::ecs::Actor::destroy(){
	for (size_t i = 0; i < components.size(); i++) {
		auto& comp = components[i];
		comp->destroy();
	}
	for (size_t i = 0; i < components.size(); i++) {
		auto& comp = components[i];
		delete comp;
	}
}
