template<typename T>
inline T * qb::ecs::Actor::addComponent() {
	T* t = new T();
	qb::ecs::ActorComponent* comp = t;
	comp->init(this);
	this->components.push_back(comp);
	return t;
}

template<typename T>
inline T* qb::ecs::Actor::removeComponent() {
	T* result = getComponent<T>();
	components.erase(std::remove(components.begin(), components.end(), result), components.end());
	result->parent = nullptr;
}

template<typename T>
inline T* qb::ecs::Actor::getComponent() {
	T* result = nullptr;
	for (size_t i = 0; i < components.size(); i++) {
		auto& comp = components[i];
		if (result = dynamic_cast<T*>(comp)) {
			break;
		}
	}
	return result;
}

template<typename T>
inline std::vector<T*> qb::ecs::Actor::removeComponents() {
	std::vector<T*> result = getComponents<T>();
	for (size_t i = 0; i < result.size(); i++) {
		components.erase(std::remove(components.begin(), components.end(), result[i]), components.end());
		result[i]->parent = nullptr;
	}
}

template<typename T>
inline std::vector<T*> qb::ecs::Actor::getComponents() {
	std::vector<T*> result;
	for (size_t i = 0; i < components.size(); i++) {
		auto& comp = components[i];
		if (result = dynamic_cast<T*>(comp))
			result.push_back(comp);
	}
	return result;
}