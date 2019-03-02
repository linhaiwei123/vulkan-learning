#include "actorComponent.h"

void qb::ecs::ActorComponent::init(qb::ecs::Actor* actor){
	this->actor = actor;
	this->active = true;
	this->uuid = boost::uuids::random_generator()();
	this->onInit();
}

void qb::ecs::ActorComponent::update(float dt){
	this->onUpdate(dt);
}

void qb::ecs::ActorComponent::destroy(){
	this->onDestroy();
}

void qb::ecs::ActorComponent::onInit(){

}

void qb::ecs::ActorComponent::onUpdate(float dt){

}

void qb::ecs::ActorComponent::onDestroy(){

}
