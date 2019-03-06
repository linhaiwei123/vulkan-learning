#pragma once
#include <vector>
#include <unordered_map>
#include <actor/actor.h>
namespace qb {
	class App;
	class Scene;
	class SceneMgr {
	private:
		std::unordered_map<std::string, qb::Scene*> _sceneMap;
	public:
		App* app;
		Scene* currentScene = nullptr;
	public:
		void init(App* app);
		qb::Scene* getScene(std::string name);
		void destroy();
	};
	class Scene {
	private:
		App* app;
		std::string name;
		std::vector<qb::ecs::Actor*> _actors;
	public:
		void init(App* app, std::string name);
		void update(float dt);
		void addActor(qb::ecs::Actor* actor);
		void destroy();
	};
}