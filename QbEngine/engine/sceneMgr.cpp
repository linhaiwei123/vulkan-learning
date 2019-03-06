#include "sceneMgr.h"
#include "app.h"
void qb::SceneMgr::init(App * app){
	this->app = app;
	// init default scene
	qb::Scene* defaultScene = this->getScene("$defaultScene");
	this->currentScene = defaultScene;
}

qb::Scene * qb::SceneMgr::getScene(std::string name){
	// read scene from cache
	auto it = _sceneMap.find(name);
	if (it != _sceneMap.end())
		return it->second;
	qb::Scene* scene = new qb::Scene();
	scene->init(app, name);
	_sceneMap.insert({ name, scene });
	return _sceneMap.at(name);
}

void qb::SceneMgr::destroy(){
	for (auto& it : _sceneMap) {
		it.second->destroy();
	}
	for (auto& it : _sceneMap) {
		delete it.second;
	}
}

void qb::Scene::init(App* app, std::string name){
	this->app = app;
	this->name = name;
}

void qb::Scene::update(float dt){
	for (auto& it : _actors) {
		if (it->active) {
			it->update(dt);
		}
	}
}

void qb::Scene::addActor(qb::ecs::Actor * actor){
	for (size_t i = 0; i < _actors.size(); i++) {
		if (_actors[i] == actor) {
			log_warn("actor already in scene, cannot add again");
			return;
		}
	}
	_actors.push_back(actor);
}

void qb::Scene::destroy(){
	for (auto& it : _actors) {
		it->destroy();
	}
	for (auto& it : _actors) {
		delete it;
	}
}
