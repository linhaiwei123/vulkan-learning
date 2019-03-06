#include "renderMgr.h"
#include "app.h"
void qb::RendeRgr::init(App* app){
	this->app = app;
}

void qb::RendeRgr::initPbrRender(){
	qb::Render* render = this->getRender("pbr");
	
}

qb::Render * qb::RendeRgr::getRender(std::string name){
	// read render from cache
	auto it = _renderMap.find(name);
	if (it != _renderMap.end())
		return it->second;
	Render* render = new Render();
	render->init(app, name);
	_renderMap.insert({ name, render });
	return _renderMap.at(name);
}

void qb::RendeRgr::destroy(){
	for (auto&it : _renderMap) {
		it.second->destroy();
	}
	for (auto& it : _renderMap) {
		delete it.second;
	}
}

void qb::Render::init(App * app, std::string name){
	this->app = app;
	this->name = name;
}

void qb::Render::destroy(){

}
