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
#include "win.h"
#include "inst.h"
#include "surface.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
#include "buffer.h"
#include "renderpass.h"
#include "sync.h"
#include "tool.h"
#include "descriptor.h"
#include "model.h"

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
	public:
		App() {};
		void run();
		void init();
		void loop();
		void draw();
		void destroy();
		virtual void onInit();
		virtual void onLoop();
		virtual void onCmd(size_t i);
		virtual void onDestroy();
	};
};