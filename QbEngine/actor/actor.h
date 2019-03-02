#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "actorComponent.h"
namespace qb {
	namespace ecs {
		class Actor {
		public:
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
			template<typename T=ecs::ActorComponent>
			T* addComponent();
			
			template<typename T=ecs::ActorComponent>
			T* removeComponent();

			template<typename T=ecs::ActorComponent>
			T* getComponent();

			template<typename T=ecs::ActorComponent>
			std::vector<T*> removeComponents();

			template<typename T=ecs::ActorComponent>
			std::vector<T*> getComponents();

			void init();
			void update(float dt);
			void destroy();
		};

	}
}