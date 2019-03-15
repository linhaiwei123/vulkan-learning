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
	log_info("model mgr initialize"); this->modelMgr.init(this);
	log_info("sync initialize"); this->sync.init(this);
	log_info("physics mgr initialize"); this->physicsMgr.init(this);
	log_info("font mgr initialize") this->fontMgr.init(this);
	log_info("audio mgr initialize") this->audioMgr.init(this);
	log_info("event mgr initialize") this->eventMgr.init(this);
	log_info("input mgr initialize"); this->inputMgr.init(this);
	log_info("actor mgr initialize") this->actorMgr.init(this);
	log_info("scene mgr initialize") this->sceneMgr.init(this);
	log_info("timer mgr initialize") this->timerMgr.init(this);
	log_info("dynamic struct mgr initialize") this->dynamicStructMgr.init(this);
	log_info("file mgr initialize") this->fileMgr.init(this);
	log_info("sub class initialize"); this->onInit();
}

void qb::App::loop() {
	this->win.mainLoop([this](){
		this->beginFrame();
		this->updateTimer();
		this->updateLogic();
		this->updatePhysics();
		this->updateAudio();
		this->updateRender();
		this->endFrame();
	});
}

void qb::App::beginFrame(){
	vkWaitForFences(this->device.logical, 1, &this->sync.inFlightFences[this->sync.currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	VkResult result = vkAcquireNextImageKHR(this->device.logical, this->swapchain.swapchain, std::numeric_limits<uint64_t>::max(), this->sync.imageAvailableSemaphores[this->sync.currentFrame], VK_NULL_HANDLE, &this->sync.currentImage);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		// recreateSwapChain(); TODO
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}
}

void qb::App::endFrame(){
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

	VkResult result = vkQueuePresentKHR(this->device.queues.present, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->win.resized) {
		this->win.resized = false;
		// recreateSwapChain(); TODO
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	this->sync.currentFrame = (this->sync.currentFrame + 1) % this->sync.maxFrames;
}

void qb::App::updateTimer(){
	this->timerMgr.update(); // timer update
}

void qb::App::updatePhysics(){
	this->physicsMgr.update(this->timerMgr.getDeltaTime()); // physics update
}

void qb::App::updateAudio(){
	this->audioMgr.update(); // audio update
}

void qb::App::updateLogic(){
	this->sceneMgr.currentScene->update(this->timerMgr.getDeltaTime());
	this->onLoop(); // logic update
}

void qb::App::updateRender(){
	this->descriptorMgr.update(); // render update
	this->bufferMgr.update();
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
	log_info("sub class destroy") this->onDestroy();
	log_info("file mgr destroy") this->fileMgr.destroy();
	log_info("dynamic struct mgr destroy") this->dynamicStructMgr.destroy();
	log_info("timer mgr destroy") this->timerMgr.destroy();
	log_info("scene mgr destroy") this->sceneMgr.destroy();
	log_info("actor mgr destroy") this->actorMgr.destroy();
	log_info("input mgr destroy") this->inputMgr.destroy();
	log_info("event mgr destroy") this->eventMgr.destroy();
	log_info("audio mgr destroy") this->audioMgr.destroy();
	log_info("font mgr destroy") this->fontMgr.destroy();
	log_info("physics mgr destroy") this->physicsMgr.destroy();
	log_info("sync destroy"); this->sync.destroy();
	log_info("model mgr destroy"); this->modelMgr.destroy();
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