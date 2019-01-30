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
	// attachment ref
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	this->attachmentRefs = { {} };
	this->attachmentRefs[0].push_back(colorAttachmentRef);
	// sub pass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &this->attachmentRefs[0][0];
	this->subpassDescs.push_back(subpass);
}

void qb::RenderPass::build() {
	// render pass
	renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescs.size());
	renderPassInfo.pSubpasses = subpassDescs.data();

	vk_check(vkCreateRenderPass(app->device.logical, &renderPassInfo, nullptr, &renderPass));
}

void qb::RenderPass::begin(size_t i){
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = app->bufferMgr.framebuffers[i];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = app->swapchain.extent;

	assert(this->clearValues.size() == renderPassInfo.attachmentCount);
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(this->clearValues.size());
	renderPassBeginInfo.pClearValues = this->clearValues.data();

	vkCmdBeginRenderPass(app->bufferMgr.commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void qb::RenderPass::end(size_t i){
	vkCmdEndRenderPass(app->bufferMgr.commandBuffers[i]);
}

void qb::RenderPass::destroy() {
	vkDestroyRenderPass(app->device.logical, renderPass, nullptr);
}
