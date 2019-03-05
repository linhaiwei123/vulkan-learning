#include "actor.h"
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "engine/app.h"
void qb::ecs::Actor::init(App * app){
	this->app = app;
	this->transform.position = glm::vec3(0.0f);
	this->transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	this->transform.scale = glm::vec3(1.0f);
	this->name = "";
	this->uuid = boost::uuids::random_generator()();
	this->active = true;
	this->eventEmitter = this->app->eventMgr.getEventEmitter(boost::lexical_cast<std::string>(this->uuid));
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
	this->eventEmitter = nullptr;
}
