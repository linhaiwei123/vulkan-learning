#include "renderpass.h"
#include "app.h"
void qb::RenderPassMgr::init(App * app){
	this->app = app;
}

qb::RenderPass* qb::RenderPassMgr::getRenderPass(std::string name) {
	// read render pass from cache
	auto it = _renderPassMap.find(name);
	if (it != _renderPassMap.end())
		return it->second;
	RenderPass* renderPass = new RenderPass();
	renderPass->init(app, name);
	_renderPassMap.insert({ name, renderPass });
	return _renderPassMap.at(name);
}

void qb::RenderPassMgr::destroy(){
	for (auto it = _renderPassMap.begin(); it != _renderPassMap.end(); it++) {
		it->second->destroy();
		delete it->second;
	}
}

void qb::RenderPass::init(App * app, std::string name) {
	this->app = app;
	this->name = name;
	// attachment desc
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = app->swapchain.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	this->attachmentDescs.push_back(colorAttachment);
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = app->device.depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	this->attachmentDescs.push_back(depthAttachment);
	// attachment ref
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	this->attachmentRefs = { {} };
	this->attachmentRefs[0].push_back(colorAttachmentRef);
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	this->attachmentRefs[0].push_back(depthAttachmentRef);
	// sub pass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &this->attachmentRefs[0][0];
	subpass.pDepthStencilAttachment = &this->attachmentRefs[0][1];
	this->subpassDescs.push_back(subpass);

	// dependency
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	this->dependencies.push_back(dependency);

}

void qb::RenderPass::build() {
	// render pass
	renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescs.size());
	renderPassInfo.pSubpasses = subpassDescs.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	vk_check(vkCreateRenderPass(app->device.logical, &renderPassInfo, nullptr, &renderPass));
}

void qb::RenderPass::begin(size_t i, VkCommandBuffer* cmdBuf){
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	assert(framebuffer != nullptr);
	renderPassBeginInfo.framebuffer = framebuffer->framebuffers[i];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = { framebuffer->createInfo.width, framebuffer->createInfo.height };

	assert(this->clearValues.size() == renderPassInfo.attachmentCount);
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(this->clearValues.size());
	renderPassBeginInfo.pClearValues = this->clearValues.data();

	if (cmdBuf == nullptr) {
		cmdBuf = &app->bufferMgr.commandBuffers[i];
	}

	vkCmdBeginRenderPass(*cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void qb::RenderPass::end(size_t i, VkCommandBuffer* cmdBuf){
	if (cmdBuf == nullptr) {
		cmdBuf = &app->bufferMgr.commandBuffers[i];
	}
	vkCmdEndRenderPass(*cmdBuf);
}

void qb::RenderPass::destroy() {
	vkDestroyRenderPass(app->device.logical, renderPass, nullptr);
}
