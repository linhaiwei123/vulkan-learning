#pragma once
#include "actorComponent.h"
namespace qb {
	namespace ecs {
		class RenderComponent : public ActorComponent {
		public:
			
		public:
			virtual void onInit();
			virtual void onUpdate(float dt);
			virtual void onDestroy();
		};
	}
}