#include "pipeline.h"
#include "app.h"

std::vector<char> qb::PipelineMgr::_readShaderCode(const std::string filename) {
	auto path = get_asset_full_path(filename);
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	assert(file.is_open());
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

VkShaderModule qb::PipelineMgr::_getShadermodule(const std::string filename) {
	// read shader module from cache
	auto it = _shadermoduleMap.find(filename);
	if (it != _shadermoduleMap.end())
		return it->second;
	// create shader module
	auto code = _readShaderCode(filename);
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shadermodule;
	vk_check(vkCreateShaderModule(app->device.logical, &createInfo, nullptr, &shadermodule));
	_shadermoduleMap.insert({ filename, shadermodule });

	return _shadermoduleMap.at(filename);
}

VkPipelineShaderStageCreateInfo qb::PipelineMgr::getShaderStage(const std::string filename, VkShaderStageFlagBits stage) {
	// read shader shader stage from cache
	auto key = std::make_pair(filename, stage);
	auto it = _shaderStageMap.find(key);
	if (it != _shaderStageMap.end())
		return it->second;
	// create shader stage
	VkPipelineShaderStageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.stage = stage;
	createInfo.module = _getShadermodule(filename);
	createInfo.pName = "main";
	_shaderStageMap.insert({ key,createInfo });
	
	return _shaderStageMap.at(key);
}

qb::GraphicsPipeline* qb::PipelineMgr::getGraphicsPipeline(const std::string name) {
	// read graphics pipeline from cache
	auto it = _graphicsPipelineMap.find(name);
	if (it != _graphicsPipelineMap.end())
		return it->second;
	GraphicsPipeline* pipeline = new GraphicsPipeline();
	pipeline->init(app, name);
	_graphicsPipelineMap.insert({ name, pipeline });

	return _graphicsPipelineMap.at(name);
}

qb::ComputePipeline * qb::PipelineMgr::getComputePipeline(const std::string name){
	// read compute pipeline from cache
	auto it = _computePipelineMap.find(name);
	if (it != _computePipelineMap.end())
		return it->second;
	ComputePipeline* pipeline = new ComputePipeline();
	pipeline->init(app, name);
	_computePipelineMap.insert({ name, pipeline });

	return _computePipelineMap.at(name);
}



void qb::PipelineMgr::init(App * app) {
	this->app = app;
	// pipeline cache
	{
		VkPipelineCacheCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		vk_check(vkCreatePipelineCache(app->device.logical, &createInfo, nullptr, &pipelineCache));
	}
}

void qb::PipelineMgr::destroy() {
	for (auto it = _shadermoduleMap.begin(); it != _shadermoduleMap.end(); it++) {
		vkDestroyShaderModule(app->device.logical, it->second, nullptr);
	}
	for (auto it = _graphicsPipelineMap.begin(); it != _graphicsPipelineMap.end(); it++) {
		it->second->destroy();
		delete it->second;
	}
	for (auto it = _computePipelineMap.begin(); it != _computePipelineMap.end(); it++) {
		it->second->destroy();
		delete it->second;
	}
	vkDestroyPipelineCache(app->device.logical, pipelineCache, nullptr);
}

void qb::GraphicsPipeline::init(App * app, std::string name) {
	this->app = app;
	this->name = name;
	// shader stage
	// vertex input
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;
	// input assembly
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	// view port
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)app->swapchain.extent.width;
	viewport.height = (float)app->swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	//viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	scissor.offset = {0, 0};
	scissor.extent = app->swapchain.extent;
	//viewportState.pScissors = &scissor;
	// rasterizer 
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	// multisampling
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;
	// depth stencil
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional
	// color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachments.push_back(colorBlendAttachment);
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
	// dynamic state
	// dynamic.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	// dynamic.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
	dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamic.size());
	dynamicState.pDynamicStates = dynamic.data();
	// pipeline layout
	pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

}

void qb::GraphicsPipeline::build() {
	assert(renderPass != VK_NULL_HANDLE);
	if (pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(app->device.logical, pipeline, nullptr);
	}
	if (layout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(app->device.logical, layout, nullptr);
	}
	vk_check(vkCreatePipelineLayout(app->device.logical, &pipelineLayoutInfo, nullptr, &layout));
	// pipeline create info
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = renderPass;
	// pipelineInfo.subpass = 0; init outside
	vk_check(vkCreateGraphicsPipelines(app->device.logical, app->pipelineMgr.pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));
}

void qb::GraphicsPipeline::destroy() {
	vkDestroyPipeline(app->device.logical, pipeline, nullptr);
	vkDestroyPipelineLayout(app->device.logical, layout, nullptr);
}

void qb::ComputePipeline::init(App * app, std::string name){
	this->app = app;
	this->name = name;
	// shader stage
	// pipeline layout
	pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
}

void qb::ComputePipeline::build(){
	if (pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(app->device.logical, pipeline, nullptr);
	}
	if (layout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(app->device.logical, layout, nullptr);
	}
	vk_check(vkCreatePipelineLayout(app->device.logical, &pipelineLayoutInfo, nullptr, &layout));
	// pipeline create info
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.stage = shaderStage;
	pipelineInfo.layout = layout;
	vk_check(vkCreateComputePipelines(app->device.logical, app->pipelineMgr.pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));
}

void qb::ComputePipeline::destroy(){
	vkDestroyPipeline(app->device.logical, pipeline, nullptr);
	vkDestroyPipelineLayout(app->device.logical, layout, nullptr);
}
