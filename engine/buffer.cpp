#include "buffer.h"
#include "app.h"
#include <vulkan/vulkan_core.h>

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
		//std::vector<VkImageView> attachments = {  //TODO separate attachments binding
		//	app->swapchain.views[i]
		//};
		assert(onAttachments != nullptr);
		std::vector<VkImageView> attachments = onAttachments(i);
		assert(attachments.size() != 0);
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

	//image destroy
	for (auto&it : _imageMap) {
		it.second->destroy();
		delete it.second;
	}
		
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

VkCommandBuffer qb::BufferMgr::beginOnce() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = app->bufferMgr.commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuf;
	vkAllocateCommandBuffers(app->device.logical, &allocInfo, &cmdBuf);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuf, &beginInfo);

	return std::forward<VkCommandBuffer>(cmdBuf);
}

void qb::BufferMgr::endOnce(VkCommandBuffer cmdBuf) {
	vkEndCommandBuffer(cmdBuf);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuf;

	vkQueueSubmit(app->device.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(app->device.queues.graphics);

	vkFreeCommandBuffers(app->device.logical, app->bufferMgr.commandPool, 1, &cmdBuf);
}

void qb::Buffer::copyToImage(Image * img){
	assert(img != nullptr);
	gli::texture* tex = img->tex;
	assert(tex != nullptr);
	// copy buffer to image
	std::vector<VkBufferImageCopy> bufferCopyRegions(static_cast<uint32_t>(tex->levels()));
	uint32_t offset = 0;
	for (uint32_t i = 0; i < bufferCopyRegions.size(); i++) {
		VkBufferImageCopy& bufferCopyRegion = bufferCopyRegions[i];
		bufferCopyRegion.imageSubresource.aspectMask = img->viewInfo.subresourceRange.aspectMask;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = static_cast<uint32_t>(tex->base_layer());
		bufferCopyRegion.imageSubresource.layerCount = static_cast<uint32_t>(tex->layers());
		bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex->extent(i).x);
		bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex->extent(i).y);
		bufferCopyRegion.imageExtent.depth = static_cast<uint32_t>(tex->extent(i).z);
		bufferCopyRegion.bufferOffset = offset;
		offset += static_cast<uint32_t>(tex->size(i));
	}

	// begin cmd
	VkCommandBuffer cmdBuf = this->app->bufferMgr.beginOnce();
	app->sync.setImageLayout(cmdBuf, img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	vkCmdCopyBufferToImage(cmdBuf, this->buffer(), img->image, img->imgLayout, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());
	app->sync.setImageLayout(cmdBuf, img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	// end cmd
	this->app->bufferMgr.endOnce(cmdBuf);
}

void qb::Image::init(App * app, std::string name){
	this->app = app;
	this->name = name;
	this->stageBuffer = app->bufferMgr.getBuffer("$tex_" + name);
}

void qb::Image::build(){
	assert(tex != nullptr);
	gli::texture::target_type target = tex->target();
	switch (target) {
	case gli::texture::target_type::TARGET_2D:
		this->_buildTex2d();
		break;
	default:
		log_error("not build handle support texture type: %d", target);
		assert(0);
	}
}

void qb::Image::buildDepth(){
	// image
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent = {
		static_cast<uint32_t>(this->app->swapchain.extent.width),
		static_cast<uint32_t>(this->app->swapchain.extent.height),
		1,
	};
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = this->app->device.depthFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = imgLayout;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;
	vk_check(vkCreateImage(app->device.logical, &imageInfo, nullptr, &image));
	// memory
	mem = app->device.allocImageMemory(image,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// view
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = imageInfo.format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	vk_check(vkCreateImageView(app->device.logical, &viewInfo, nullptr, &view));

	auto cmdBuf = this->app->bufferMgr.beginOnce();
	this->app->sync.setImageLayout(cmdBuf, this, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
	this->app->bufferMgr.endOnce(cmdBuf);
}

void qb::Image::_buildTex2d() {
	// image
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent = {
		static_cast<uint32_t>(tex->extent(0).x),
		static_cast<uint32_t>(tex->extent(0).y),
		static_cast<uint32_t>(tex->extent(0).z),
	};
	imageInfo.mipLevels = static_cast<uint32_t>(tex->levels());
	imageInfo.arrayLayers = static_cast<uint32_t>(tex->layers());
	imageInfo.format = static_cast<VkFormat>(tex->format());
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = imgLayout;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;
	vk_check(vkCreateImage(app->device.logical, &imageInfo, nullptr, &image));
	// memory
	mem = app->device.allocImageMemory(image,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// view
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = imageInfo.format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = static_cast<uint32_t>(tex->base_level());
	viewInfo.subresourceRange.levelCount = static_cast<uint32_t>(tex->levels());
	viewInfo.subresourceRange.baseArrayLayer = static_cast<uint32_t>(tex->base_layer());
	viewInfo.subresourceRange.layerCount = static_cast<uint32_t>(tex->layers());

	vk_check(vkCreateImageView(app->device.logical, &viewInfo, nullptr, &view));

	// stage buffer
	this->stageBuffer->bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	this->stageBuffer->bufferInfo.size = tex->size();
	this->stageBuffer->build();
	this->stageBuffer->mapping(tex->data());
	this->stageBuffer->copyToImage(this);

	// sampler
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(tex->levels());
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	vk_check(vkCreateSampler(app->device.logical, &samplerInfo, nullptr, &sampler));

	// descriptor image info
	descriptorImageInfo.imageLayout = imgLayout;
	descriptorImageInfo.imageView = view;
	descriptorImageInfo.sampler = sampler;
}

void qb::Image::destroy(){
	vkDestroyImageView(app->device.logical, view, nullptr);
	vkDestroyImage(app->device.logical, image, nullptr);
	vkDestroySampler(app->device.logical, sampler, nullptr);
	vkFreeMemory(app->device.logical, mem, nullptr);
}

VkDescriptorSetLayoutBinding qb::Image::getLayoutBinding(uint32_t binding, VkShaderStageFlags stageFlags){
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = binding;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = stageFlags;
	return samplerLayoutBinding;
}
