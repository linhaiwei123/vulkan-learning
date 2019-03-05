#include "ActorMgr.h"

void qb::ActorMgr::init(App * app){
	this->app = app;
}

qb::ecs::Actor * qb::ActorMgr::getActor() {
	qb::ecs::Actor* actor = new qb::ecs::Actor();
	actor->init(this->app);
	this->_actors.push_back(actor);
	return actor;
}

void qb::ActorMgr::destroy(){
	for (auto& it : _actors) {
		it->destroy();
	}
	for (auto& it : _actors) {
		delete it;
	}
}
