#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
#include "PxPhysicsAPI.h"

namespace qb {
	class App;
	class PhysicsMgr {
	public: //physx
		physx::PxDefaultAllocator pxAllocator;
		physx::PxDefaultErrorCallback pxErrorCallback;
		physx::PxFoundation* pxFoundation;
		physx::PxPhysics* pxPhysics;
		physx::PxDefaultCpuDispatcher* pxDispatcher;
		physx::PxScene* pxScene;
		physx::PxMaterial* pxMaterial;
	public:
		App *app;
	public:
		PhysicsMgr() = default;

		void init(App *app);

		void update(float dt);

		void destroy();
	};
};