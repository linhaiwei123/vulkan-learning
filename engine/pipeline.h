#pragma once
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <fstream>
#include <vulkan/vulkan.h>
namespace qb {
	class App;
	class GraphicsPipeline;
	class PipelineMgr {
	private:
		std::unordered_map<std::string, VkShaderModule> _shadermoduleMap{};
		std::unordered_map<std::string, GraphicsPipeline*> _graphicsPipelineMap{};
		std::map<std::pair<std::string, VkShaderStageFlagBits>, VkPipelineShaderStageCreateInfo> _shaderStageMap{};
	public:
		App *app;
		VkPipelineCache pipelineCache = VK_NULL_HANDLE;
	private:
		std::vector<char> _readShaderCode(const std::string filename);
		VkShaderModule _getShadermodule(const std::string filename);
	public:
		PipelineMgr() = default;
		
		VkPipelineShaderStageCreateInfo getShaderStage(const std::string filename, VkShaderStageFlagBits stage);
		GraphicsPipeline* getGraphicsPipeline(const std::string name);
		
		void init(App *app);
		void destroy();
	};

	class GraphicsPipeline {
	private:
		App* app;
		std::string name;
	public:
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		VkPipelineViewportStateCreateInfo viewportState{};
		VkViewport viewport{};
		VkRect2D scissor{};
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		VkPipelineMultisampleStateCreateInfo multisampling{};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		std::vector<VkDynamicState> dynamic{};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		VkPipelineLayout layout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkRenderPass renderPass = VK_NULL_HANDLE;
	public:
		GraphicsPipeline() = default;

		void init(App* app, std::string name);
		void build();
		void destroy();
	};
};