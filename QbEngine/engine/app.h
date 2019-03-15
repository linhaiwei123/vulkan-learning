#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
#include <chrono>
#include <gli/gli.hpp>
#include "json.hpp"
#include "win.h"
#include "inst.h"
#include "surface.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
#include "bufferMgr.h"
#include "renderpass.h"
#include "sync.h"
#include "tool.h"
#include "descriptorMgr.h"
#include "modelMgr.h"
#include "physicsMgr.h"
#include "fontMgr.h"
#include "audioMgr.h"
#include "eventMgr.h"
#include "inputMgr.h"
#include "actorMgr.h"
#include "sceneMgr.h"
#include "timerMgr.h"
#include "dynamicStructMgr.h"
#include "fileMgr.h"

namespace qb {
	class App {
	public:
		Win win;
		Inst inst;
		Device device;
		Surf surf;
		Swapchain swapchain;
		PipelineMgr pipelineMgr;
		RenderPassMgr renderPassMgr;
		BufferMgr bufferMgr;
		Sync sync;
		DescriptorMgr descriptorMgr;
		ModelMgr modelMgr;
		PhysicsMgr physicsMgr;
		FontMgr fontMgr;
		AudioMgr audioMgr;
		EventMgr eventMgr;
		InputMgr inputMgr;
		ActorMgr actorMgr;
		SceneMgr sceneMgr;
		TimerMgr timerMgr;
		DynamicStructMgr dynamicStructMgr;
		FileMgr fileMgr;
	public:
		App() {};
		void run();
		void init();
		void loop();
		void beginFrame();
		void endFrame();
		void updateTimer();
		void updatePhysics();
		void updateAudio();
		void updateLogic();
		void updateRender();
		void destroy();
		virtual void onInit();
		virtual void onLoop();
		virtual void onCmd(size_t i);
		virtual void onDestroy();
	};
};