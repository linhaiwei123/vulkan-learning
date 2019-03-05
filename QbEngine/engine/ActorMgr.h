#pragma once
#include "actor/actor.h"
#include <vector>
namespace qb {
	class App;
	class ActorMgr {	
	private:
		std::vector<qb::ecs::Actor*> _actors;
	public:
		App* app;
	public:
		void init(App* app);
		qb::ecs::Actor* getActor();
		void destroy();
	};
}