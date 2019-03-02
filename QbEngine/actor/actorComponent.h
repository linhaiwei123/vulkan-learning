#pragma once
#include "actor.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
namespace qb {
	namespace ecs {
		class qb::ecs::Actor;
		class ActorComponent {
		public:
			qb::ecs::Actor* actor;
			bool active;
			boost::uuids::uuid uuid;
		public:
			void init(qb::ecs::Actor* actor);
			void update(float dt);
			void destroy();

			virtual void onInit();
			virtual void onUpdate(float dt);
			virtual void onDestroy();
		};
	}
}