#include "descriptor.h"
#include "app.h"
#include <array>
void qb::DescriptorMgr::init(App *app) {
	this->app = app;

	//descriptor pool
	uint32_t descriptorCount = static_cast<uint32_t>(app->swapchain.views.size()) * 1000;
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCount},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptorCount},
		{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, descriptorCount},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptorCount},
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(descriptorCount * poolSizes.size());

	vk_check(vkCreateDescriptorPool(app->device.logical, &poolInfo, nullptr, &descriptorPool));
}

qb::Descriptor* qb::DescriptorMgr::getDescriptor(std::string name){
	// read descriptor from cache
	auto it = _descriptorMap.find(name);
	if (it != _descriptorMap.end())
		return it->second;
	Descriptor* descriptor = new Descriptor();
	descriptor->init(app, name);
	_descriptorMap.insert({ name, descriptor });
	return _descriptorMap.at(name);
}

void qb::DescriptorMgr::destroy() {
	for (auto it = _descriptorMap.begin(); it != _descriptorMap.end(); it++) {
		it->second->destroy();
		delete it->second;
	}
	vkDestroyDescriptorPool(app->device.logical, descriptorPool, nullptr);
}

void qb::DescriptorMgr::update(){
	for (auto& it : _descriptorMap) {
		it.second->update();
	}
}

bool qb::Descriptor::is_descriptor_type_image(VkDescriptorType type){
	if (type == VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER ||
		type == VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
		type == VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
		type == VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
		type == VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
		)
		return true;
		return false;
}

bool qb::Descriptor::is_descriptor_type_texel_buffer(VkDescriptorType type){
	if (type == VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
		type == VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
		)
		return true;
		return false;
}

bool qb::Descriptor::is_descriptor_type_uniform(VkDescriptorType type){
	if (type == VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
		type == VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
		type == VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
		type == VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
		)
		return true;
		return false;
}

void qb::Descriptor::init(App * app, std::string name){
	this->app = app;
	this->name = name;
	this->_descriptorSetDirtyNum = 0;
}

void qb::Descriptor::build(size_t count){
	assert(count >= 1);
	assert(bindings.size() != 0);
	// descriptor set layout
	layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutBindings.resize(bindings.size());
	transform(bindings.begin(), bindings.end(), layoutBindings.begin(), [](auto&v) -> VkDescriptorSetLayoutBinding& {return v.first; });
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();
	vkCreateDescriptorSetLayout(app->device.logical, &layoutInfo, nullptr, &layout);
	// descriptor set
	std::vector<VkDescriptorSetLayout> layouts(count, layout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = app->descriptorMgr.descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(count);
	allocInfo.pSetLayouts = layouts.data();
	descriptorSets.resize(count);
	vk_check(vkAllocateDescriptorSets(app->device.logical, &allocInfo, descriptorSets.data()));

	// descriptor buffer/image info
	std::vector<void*> buffers(bindings.size());
	transform(bindings.begin(), bindings.end(), buffers.begin(), [&](auto&v) {return v.second; });
	size_t bufferCount = buffers.size();
	for (size_t i = 0; i < count; i++) {
		std::vector<VkWriteDescriptorSet> writeSets(bufferCount);
		for (size_t j = 0; j < bufferCount; j++) {
			VkWriteDescriptorSet& descriptorWrite = writeSets[j];
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = layoutBindings[j].binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.descriptorType = layoutBindings[j].descriptorType;
			if(is_descriptor_type_uniform(descriptorWrite.descriptorType))
				descriptorWrite.pBufferInfo = &reinterpret_cast<Buffer*>(buffers[j])->descriptorBufferInfos[i];
			else if(is_descriptor_type_image(descriptorWrite.descriptorType))
				descriptorWrite.pImageInfo = &reinterpret_cast<Image*>(buffers[j])->descriptorImageInfo;
			else if(is_descriptor_type_texel_buffer(descriptorWrite.descriptorType))
				descriptorWrite.pTexelBufferView = nullptr; // TODO
			else assert(0);
		}
		vkUpdateDescriptorSets(app->device.logical, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	}
}

void qb::Descriptor::buildPerSwapchainImg(){
	this->build(app->swapchain.views.size());
}

void qb::Descriptor::setDescriptorSetDirty(){
	this->_descriptorSetDirtyNum = app->swapchain.views.size();
	app->bufferMgr.setCommandBufferDirty();
}

void qb::Descriptor::update(){
	if (this->_descriptorSetDirtyNum > 0) {
		this->_descriptorSetDirtyNum--;

		size_t currImg = this->app->sync.currentImage;

		std::vector<void*> buffers(bindings.size());
		transform(bindings.begin(), bindings.end(), buffers.begin(), [&](auto&v) {return v.second; });
		size_t bufferCount = buffers.size();

		std::vector<VkWriteDescriptorSet> writeSets(bufferCount);
		for (size_t j = 0; j < bufferCount; j++) {
			VkWriteDescriptorSet& descriptorWrite = writeSets[j];
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[currImg];
			descriptorWrite.dstBinding = layoutBindings[j].binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.descriptorType = layoutBindings[j].descriptorType;
			if (is_descriptor_type_uniform(descriptorWrite.descriptorType))
				descriptorWrite.pBufferInfo = &reinterpret_cast<Buffer*>(buffers[j])->descriptorBufferInfos[currImg];
			else if (is_descriptor_type_image(descriptorWrite.descriptorType))
				descriptorWrite.pImageInfo = &reinterpret_cast<Image*>(buffers[j])->descriptorImageInfo;
			else if (is_descriptor_type_texel_buffer(descriptorWrite.descriptorType))
				descriptorWrite.pTexelBufferView = nullptr; // TODO
			else assert(0);
		}
		vkUpdateDescriptorSets(app->device.logical, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	}
}

void qb::Descriptor::destroy(){
	vkDestroyDescriptorSetLayout(app->device.logical, layout, nullptr);
}
