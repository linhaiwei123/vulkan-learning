#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
#include <optional>
#include <unordered_map>
namespace qb {
	class App;
	class RenderPass;
	class RenderPassMgr {
	private:
		std::unordered_map<std::string, RenderPass*> _renderPassMap{};
	public:
		App *app;
	public:
		RenderPassMgr() = default;

		void init(App *app);
		RenderPass* getRenderPass(std::string name);
		void destroy();
	};

	class RenderPass {
	public:
		App* app;
		std::string name;
		VkRenderPassCreateInfo renderPassInfo;
		VkRenderPass renderPass;
		std::vector<VkAttachmentDescription> attachmentDescs{};
		std::vector<std::vector<VkAttachmentReference>> attachmentRefs{};
		std::vector<VkSubpassDescription> subpassDescs{};
		std::vector<VkClearValue> clearValues{};
		std::vector<VkSubpassDependency> dependencies{};
	public:
		RenderPass() = default;
		
		void init(App* app, std::string name);
		void build();
		void begin(size_t i);
		void end(size_t i);
		void destroy();
	};
};