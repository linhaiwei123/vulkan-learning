#include "buffer.h"
#include "app.h"

void qb::BufferMgr::init(App * app) {
	this->app = app;

	// command pool
	{
		qb::Device::QueueFamilyIndices& queueFamilyIndices = app->device.queueFamilyIndices;
		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamilyIndices.graphics.value();
		createInfo.flags = 0;
		vk_check(vkCreateCommandPool(app->device.logical, &createInfo, nullptr, &commandPool));
	}
	// command buffer
	{
		commandBuffers.resize(app->swapchain.views.size());
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
		vk_check(vkAllocateCommandBuffers(app->device.logical, &allocInfo, commandBuffers.data()));
	}
}

void qb::BufferMgr::build() {
	assert(renderPass != VK_NULL_HANDLE);

	framebuffers.resize(app->swapchain.views.size());
	// frame buffer create
	for (size_t i = 0; i < framebuffers.size(); i++) {
		std::vector<VkImageView> attachments = {  //TODO separate attachments binding
			app->swapchain.views[i]
		};
		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass;
		createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		createInfo.pAttachments = attachments.data();
		createInfo.width = app->swapchain.extent.width;
		createInfo.height = app->swapchain.extent.height;
		createInfo.layers = 1;

		vk_check(vkCreateFramebuffer(app->device.logical, &createInfo, nullptr, &framebuffers[i]));
	}
	// command buffer record
	for (size_t i = 0; i < commandBuffers.size(); i++) {
		app->onCmd(i);
	}
}

void qb::BufferMgr::begin(size_t i){
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr; // Optional

	vk_check(vkBeginCommandBuffer(this->commandBuffers[i], &beginInfo));
}

void qb::BufferMgr::end(size_t i) {
	vk_check(vkEndCommandBuffer(this->commandBuffers[i]));
}

void qb::BufferMgr::destroy() {
	for (auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(app->device.logical, framebuffer, nullptr);
	}
	vkDestroyCommandPool(app->device.logical, commandPool, nullptr);

	//buffer destroy
	for (auto it = _bufferMap.begin(); it != _bufferMap.end(); it++) {
		it->second->destroy();
		delete it->second;
	}

	//texture2d destroy
	for (auto&it : _texMap)
		delete it.second;
}

qb::Buffer * qb::BufferMgr::getBuffer(std::string name) {
	// read buffer from cache
	auto it = _bufferMap.find(name);
	if (it != _bufferMap.end())
		return it->second;
	Buffer* buffer = new Buffer();
	buffer->init(app, name);
	_bufferMap.insert({ name, buffer });
	return _bufferMap.at(name);
}

qb::Image * qb::BufferMgr::getImage(std::string name) {
	// read image from cache
	auto it = _imageMap.find(name);
	if (it != _imageMap.end())
		return it->second;
	Image* img = new Image();
	img->init(app, name);
	_imageMap.insert({ name, img });
	return _imageMap.at(name);
}

gli::texture* qb::BufferMgr::getTex(std::string name){
	// read texture2d from cache
	auto it = _texMap.find(name);
	if (it != _texMap.end())
		return it->second;
	gli::texture* tex = new gli::texture(gli::load(name));
	assert(!tex->empty());
	_texMap.insert({ name, tex });
	return _texMap.at(name);
}

void qb::Buffer::init(App * app, std::string name) {
	this->app = app;
	this->name = name;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
}


void qb::Buffer::build(size_t count) {
	assert(count >= 1);
	assert(bufferInfo.usage != VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM);
	assert(bufferInfo.size != std::numeric_limits<uint64_t>::max());
	buffers.resize(count);
	mems.resize(count);
	for (size_t i = 0; i < count; i++) {
		// buffer
		vk_check(vkCreateBuffer(app->device.logical, &bufferInfo, nullptr, &buffers[i]));
		// memory
		mems[i] = app->device.allocBufferMemory(buffers[i],
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	if (descriptorRange > 0) {
		assert(count > 1);
		//descriptor buffer info
		descriptorBufferInfos.resize(count);
		for (size_t i = 0; i < count; i++) {
			VkDescriptorBufferInfo& bufferInfo = descriptorBufferInfos[i];
			bufferInfo.buffer = buffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = descriptorRange;
		}
	}
}

void qb::Buffer::buildPerSwapchainImg(){
	this->build(app->swapchain.views.size());
}

void qb::Buffer::mapping(void * data, size_t i) {
	assert(i >= 0);
	assert(i < buffers.size());
	void* addr;
	vkMapMemory(app->device.logical, mems[i], 0, bufferInfo.size, 0, &addr);
	memcpy(addr, data, bufferInfo.size);
	vkUnmapMemory(app->device.logical, mems[i]);
}

void qb::Buffer::mappingCurSwapchainImg(void * data){
	mapping(data, this->app->sync.currentImage);
}

void qb::Buffer::destroy() {
	for(auto& buffer : buffers)
		vkDestroyBuffer(app->device.logical, buffer, nullptr);
	for(auto& mem : mems)
		vkFreeMemory(app->device.logical, mem, nullptr);
}

void qb::Image::init(App * app, std::string name){
	this->app = app;
	this->name = name;
}

void qb::Image::build(){

}

void qb::Image::destroy(){

}
