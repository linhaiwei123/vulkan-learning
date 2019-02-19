#include "app.h"
void qb::App::run() {
	init();
	loop();
	destroy();
}

void qb::App::init() {
	
	log_info("win initialize"); this->win.init(this);
	log_info("inst initialize"); this->inst.init(this);
	log_info("surf initialize"); this->surf.init(this);
	log_info("device initialize"); this->device.init(this);
	log_info("swapchain initialize"); this->swapchain.init(this);
	log_info("pipeline mgr initialize"); this->pipelineMgr.init(this);
	log_info("render pass mgr initialize"); this->renderPassMgr.init(this);
	log_info("buffer mgr initialize"); this->bufferMgr.init(this);
	log_info("descriptor mgr initialize"); this->descriptorMgr.init(this);
	log_info("model initialize"); this->modelMgr.init(this);
	log_info("sync initialize"); this->sync.init(this);
	log_info("sub class initialize"); this->onInit();
}

void qb::App::loop() {
	this->win.mainLoop([this](){
		this->draw();
	});
}

void qb::App::draw() {
	vkWaitForFences(this->device.logical, 1, &this->sync.inFlightFences[this->sync.currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	VkResult result = vkAcquireNextImageKHR(this->device.logical, this->swapchain.swapchain, std::numeric_limits<uint64_t>::max(), this->sync.imageAvailableSemaphores[this->sync.currentFrame], VK_NULL_HANDLE, &this->sync.currentImage);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		// recreateSwapChain(); TODO
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	this->onLoop();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { this->sync.imageAvailableSemaphores[this->sync.currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->bufferMgr.commandBuffers[this->sync.currentImage];
	VkSemaphore signalSemaphores[] = { this->sync.renderFinishedSemaphores[this->sync.currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(this->device.logical, 1, &this->sync.inFlightFences[this->sync.currentFrame]);

	vk_check(vkQueueSubmit(this->device.queues.graphics, 1, &submitInfo, this->sync.inFlightFences[this->sync.currentFrame]));

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { this->swapchain.swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &this->sync.currentImage;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(this->device.queues.present, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->win.resized) {
		this->win.resized = false;
		// recreateSwapChain(); TODO
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	this->sync.currentFrame = (this->sync.currentFrame + 1) % this->sync.MAX_FRAMES;
}

void qb::App::onInit() {

}

void qb::App::onLoop() {

}

void qb::App::onCmd(size_t i){

}

void qb::App::onDestroy() {
	
}


void qb::App::destroy() {
	log_info("sub class destroy"); this->onDestroy();
	log_info("sync destroy"); this->sync.destroy();
	log_info("model destroy"); this->modelMgr.destroy();
	log_info("descriptor mgr destroy"); this->descriptorMgr.destroy();
	log_info("buffer mgr destroy"); this->bufferMgr.destroy();
	log_info("pipeline mgr destroy"); this->pipelineMgr.destroy();
	log_info("render pass mgr destroy"); this->renderPassMgr.destroy();
	log_info("swapchain destroy"); this->swapchain.destroy();
	log_info("device destroy"); this->device.destroy();
	log_info("surf destroy"); this->surf.destroy();
	log_info("inst destroy"); this->inst.destroy();
	log_info("win destroy"); this->win.destroy();
}