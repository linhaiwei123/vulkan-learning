#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "actorComponent.h"
namespace qb {
	class App;
	class EventEmitter;
	namespace ecs {
		class Actor {
		public:
			App* app;
			struct Transform {
				glm::vec3 position;
				glm::quat rotation;
				glm::vec3 scale;
			}transform;
			ecs::Actor* parent;
			std::vector<ecs::Actor*> children{};
			std::vector<ecs::ActorComponent*> components{};
			std::string name;
			boost::uuids::uuid uuid;
			bool active;
		public:
			// component system
			template<typename T=ecs::ActorComponent>
			inline T* addComponent();
			
			template<typename T=ecs::ActorComponent>
			inline T* removeComponent();

			template<typename T=ecs::ActorComponent>
			inline T* getComponent();

			template<typename T=ecs::ActorComponent>
			inline std::vector<T*> removeComponents();

			template<typename T=ecs::ActorComponent>
			inline std::vector<T*> getComponents();

			// node life cycle
			void init(App* app);
			void update(float dt);
			void destroy();

			// event emitter
			qb::EventEmitter* eventEmitter = nullptr;
		};

	}
}

#include "actor.ipp"