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

void qb::BufferMgr::destroyBuffer(std::string name){
	auto it = _bufferMap.find(name);
	if (it != _bufferMap.end()) {
		Buffer* buffer = it->second;
		buffer->destroy();
		delete it->second;
		_bufferMap.erase(it);
	}
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
	auto path = get_asset_full_path(name);
	gli::texture* tex = new gli::texture(gli::load(path));
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

void qb::Buffer::mapping(void * data, size_t i, size_t offset, size_t size) {
	size_t bufferInfoSize = size != 0 ? size : bufferInfo.size;
	assert(i >= 0);
	assert(i < buffers.size());
	void* addr;
	vkMapMemory(app->device.logical, mems[i], offset, bufferInfoSize, 0, &addr);
	memcpy(addr, data, bufferInfoSize);
	vkUnmapMemory(app->device.logical, mems[i]);
}

void qb::Buffer::mappingCurSwapchainImg(void * data, size_t offset, size_t size){
	mapping(data, this->app->sync.currentImage, offset, size);
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

void qb::Buffer::copyToImage(qb::Image * img){
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
	img->setImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuf);
	vkCmdCopyBufferToImage(cmdBuf, this->buffer(), img->image, img->getImageLayout(), static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());
	this->app->bufferMgr.endOnce(cmdBuf);
	// end cmd
}

void qb::Image::init(App * app, std::string name){
	this->app = app;
	this->name = name;

	// image
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_MAX_ENUM; // init outside
	imageInfo.extent = {
		static_cast<uint32_t>(this->app->swapchain.extent.width),
		static_cast<uint32_t>(this->app->swapchain.extent.height),
		1,
	};
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_MAX_ENUM; // init outside
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM; // init outside
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	// view
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = VK_NULL_HANDLE;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM; // init outside
	viewInfo.format = VK_FORMAT_MAX_ENUM; //init inside
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM; //init outside
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

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
	samplerInfo.maxLod = 1.0f;
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	// descriptor image info
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_MAX_ENUM; // init outside
	descriptorImageInfo.imageView = VK_NULL_HANDLE;// init inside
	descriptorImageInfo.sampler = VK_NULL_HANDLE; // init inside
}

void qb::Image::build() {
	// texture
	if (tex != nullptr) {
		imageInfo.imageType = _getImageTypeFromTex();
		imageInfo.extent = {
			static_cast<uint32_t>(tex->extent(0).x),
			static_cast<uint32_t>(tex->extent(0).y),
			static_cast<uint32_t>(tex->extent(0).z),
		};
		imageInfo.mipLevels = static_cast<uint32_t>(tex->levels());
		imageInfo.arrayLayers = static_cast<uint32_t>(tex->layers());
		imageInfo.format = static_cast<VkFormat>(tex->format());
		imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		// view
		viewInfo.viewType = _getImageViewTypeFromTex();
		viewInfo.subresourceRange.baseMipLevel = static_cast<uint32_t>(tex->base_level());
		viewInfo.subresourceRange.levelCount = static_cast<uint32_t>(tex->levels());
		viewInfo.subresourceRange.baseArrayLayer = static_cast<uint32_t>(tex->base_layer());
		viewInfo.subresourceRange.layerCount = static_cast<uint32_t>(tex->layers());
		viewInfo.components = _getImageViewComponentMappingFromTex();

		// sampler
		samplerInfo.maxLod = static_cast<float>(tex->levels());
	}

	// image
	assert(imageInfo.imageType != VK_IMAGE_TYPE_MAX_ENUM); // init outside
	assert(imageInfo.format != VK_FORMAT_MAX_ENUM); // init outside
	assert(imageInfo.initialLayout != VK_IMAGE_LAYOUT_MAX_ENUM); // init outside 
	assert(imageInfo.usage != VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM); // init outside
	vk_check(vkCreateImage(app->device.logical, &imageInfo, nullptr, &image));
	// memory
	mem = app->device.allocImageMemory(image,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	// view
	viewInfo.image = image;
	assert(viewInfo.viewType != VK_IMAGE_VIEW_TYPE_MAX_ENUM); // init outside
	viewInfo.format = imageInfo.format; //init inside
	assert(viewInfo.subresourceRange.aspectMask != VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM); //init outside
	vk_check(vkCreateImageView(app->device.logical, &viewInfo, nullptr, &view));

	// stage buffer
	if (tex != nullptr) {
		this->stageBuffer = app->bufferMgr.getBuffer("$tex_" + name);
		this->stageBuffer->bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		this->stageBuffer->bufferInfo.size = tex->size();
		this->stageBuffer->build();
		this->stageBuffer->mapping(tex->data());
		this->stageBuffer->copyToImage(this);
	}

	// sampler
	vk_check(vkCreateSampler(app->device.logical, &samplerInfo, nullptr, &sampler));

	// descriptor image info
	assert(descriptorImageInfo.imageLayout != VK_IMAGE_LAYOUT_MAX_ENUM); // init outside
	descriptorImageInfo.imageView = view; // init inside
	descriptorImageInfo.sampler = sampler; // init inside
}

VkImageType qb::Image::_getImageTypeFromTex(){
	if (tex == nullptr)
		return VkImageType::VK_IMAGE_TYPE_MAX_ENUM; // tex is nullptr
	gli::texture::target_type targetType = tex->target();
	switch (targetType) { 
	case gli::texture::target_type::TARGET_1D:
	case gli::texture::target_type::TARGET_1D_ARRAY:
		return VkImageType::VK_IMAGE_TYPE_1D;
	case gli::texture::target_type::TARGET_2D:
	case gli::texture::target_type::TARGET_2D_ARRAY:
	case gli::texture::target_type::TARGET_RECT:
	case gli::texture::target_type::TARGET_RECT_ARRAY:
	case gli::texture::target_type::TARGET_CUBE:
	case gli::texture::target_type::TARGET_CUBE_ARRAY:
		return VkImageType::VK_IMAGE_TYPE_2D;
	case gli::texture::target_type::TARGET_3D:
		return VkImageType::VK_IMAGE_TYPE_3D;
	default:
		assert(0);
		return VkImageType::VK_IMAGE_TYPE_MAX_ENUM;
	}
}

VkImageViewType qb::Image::_getImageViewTypeFromTex(){
	if (tex == nullptr)
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_MAX_ENUM; // tex is nullptr
	gli::texture::target_type targetType = tex->target();
	switch (targetType) {
	case gli::texture::target_type::TARGET_1D:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_1D;
	case gli::texture::target_type::TARGET_1D_ARRAY:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	case gli::texture::target_type::TARGET_2D:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
	case gli::texture::target_type::TARGET_2D_ARRAY:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case gli::texture::target_type::TARGET_RECT:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
	case gli::texture::target_type::TARGET_RECT_ARRAY:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case gli::texture::target_type::TARGET_CUBE:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;
	case gli::texture::target_type::TARGET_CUBE_ARRAY:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
	case gli::texture::target_type::TARGET_3D:
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_3D;
	default:
		assert(0);
		return VkImageViewType::VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	}
}

VkComponentMapping qb::Image::_getImageViewComponentMappingFromTex(){
	if (tex == nullptr)
		return VkComponentMapping{};
	gli::swizzles swizzles = tex->swizzles();
	auto switchMapping = [](gli::swizzle swizzle) -> VkComponentSwizzle {
		switch (swizzle) {
		case gli::swizzle::SWIZZLE_RED:
			return VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R;
		case gli::swizzle::SWIZZLE_GREEN:
			return VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G;
		case gli::swizzle::SWIZZLE_BLUE:
			return VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B;
		case gli::swizzle::SWIZZLE_ALPHA:
			return VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A;
		case gli::swizzle::SWIZZLE_ZERO:
			return VkComponentSwizzle::VK_COMPONENT_SWIZZLE_ZERO;
		case gli::swizzle::SWIZZLE_ONE:
			return VkComponentSwizzle::VK_COMPONENT_SWIZZLE_ONE;
		default:
			assert(0);
			return VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
		}
	};
	return {
		switchMapping(swizzles.r),
		switchMapping(swizzles.g),
		switchMapping(swizzles.b),
		switchMapping(swizzles.a),
	};
}

void qb::Image::destroy(){
	vkDestroyImageView(app->device.logical, view, nullptr);
	vkDestroyImage(app->device.logical, image, nullptr);
	vkDestroySampler(app->device.logical, sampler, nullptr);
	vkFreeMemory(app->device.logical, mem, nullptr);
}

void qb::Image::setImageLayout(VkImageLayout layout, VkPipelineStageFlagBits srcStage, VkPipelineStageFlagBits dstStage, VkCommandBuffer cmdBuf){

	bool cmdBufNotExist = cmdBuf == VK_NULL_HANDLE;
	if(cmdBufNotExist)
		cmdBuf = this->app->bufferMgr.beginOnce();
	this->app->sync.setImageLayout(cmdBuf, this, _imgLayout, layout, srcStage, dstStage);
	if (cmdBufNotExist)
		this->app->bufferMgr.endOnce(cmdBuf);
	_imgLayout = layout;
}

VkImageLayout qb::Image::getImageLayout(){
	return _imgLayout;
}
