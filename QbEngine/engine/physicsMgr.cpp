#include "physicsMgr.h"
#include "app.h"

void qb::Physics::init(App * app) {
	this->app = app;
	// physics
	pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, pxAllocator, pxErrorCallback);
	pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, physx::PxTolerancesScale(), true, nullptr);
	physx::PxSceneDesc sceneDesc(pxPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	pxDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = pxDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	pxScene = pxPhysics->createScene(sceneDesc);
	pxMaterial = pxPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}

void qb::Physics::step(float dt){
	pxScene->simulate(dt);
	pxScene->fetchResults(true);
}

void qb::Physics::destroy(){
	pxScene->release();
	pxDispatcher->release();
	pxPhysics->release();
	pxFoundation->release();
}
