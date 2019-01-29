#include "sync.h"
#include "app.h"
void qb::Sync::init(App * app) {
	this->app = app;
	// create semaphores
	imageAvailableSemaphores.resize(MAX_FRAMES);
	renderFinishedSemaphores.resize(MAX_FRAMES);
	inFlightFences.resize(MAX_FRAMES);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES; i++) {
		vk_check(vkCreateSemaphore(app->device.logical, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
		vk_check(vkCreateSemaphore(app->device.logical, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
		vk_check(vkCreateFence(app->device.logical, &fenceInfo, nullptr, &inFlightFences[i]));
	}
}

void qb::Sync::destroy() {
	for (size_t i = 0; i < MAX_FRAMES; i++) {
		vkDestroySemaphore(app->device.logical, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(app->device.logical, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(app->device.logical, inFlightFences[i], nullptr);
	}
}
