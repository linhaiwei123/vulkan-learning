#include "descriptor.h"
#include "app.h"
#include <array>
void qb::DescriptorMgr::init(App *app) {
	this->app = app;

	// descriptor pool
	std::array<VkDescriptorPoolSize, 4> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(app->swapchain.views.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(app->swapchain.views.size());
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(app->swapchain.views.size());
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	poolSizes[3].descriptorCount = static_cast<uint32_t>(app->swapchain.views.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(app->swapchain.views.size());

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

void qb::Descriptor::init(App * app, std::string name){
	this->app = app;
	this->name = name;
}

void qb::Descriptor::build(){
	assert(bindings.size() != 0);
	// descriptor set layout
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings(bindings.size());
	transform(bindings.begin(), bindings.end(), layoutBindings.begin(), [](auto&v) -> VkDescriptorSetLayoutBinding& {return v.first; });
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();
	vkCreateDescriptorSetLayout(app->device.logical, &layoutInfo, nullptr, &layout);
	// descriptor set
	std::vector<VkDescriptorSetLayout> layouts(app->swapchain.views.size(), layout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = app->descriptorMgr.descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(app->swapchain.views.size());
	allocInfo.pSetLayouts = layouts.data();
	descriptorSets.resize(app->swapchain.views.size());
	vk_check(vkAllocateDescriptorSets(app->device.logical, &allocInfo, descriptorSets.data()));

	// descriptor buffer/image info
	std::vector<Buffer*> buffers(bindings.size());
	transform(bindings.begin(), bindings.end(), buffers.begin(), [&](auto&v) -> Buffer* {return v.second; });
	size_t bufferCount = buffers.size();
	for (size_t i = 0; i < app->swapchain.views.size(); i++) {
		std::vector<VkWriteDescriptorSet> writeSets(bufferCount);
		for (size_t j = 0; j < bufferCount; j++) {
			VkWriteDescriptorSet& descriptorWrite = writeSets[j];
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = layoutBindings[j].binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.descriptorType = layoutBindings[j].descriptorType;
			descriptorWrite.pBufferInfo = descriptorWrite.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ? &buffers[j]->descriptorBufferInfos[i] : nullptr;
			descriptorWrite.pImageInfo = nullptr; // TODO
			descriptorWrite.pTexelBufferView = nullptr; // TODO
		}
		vkUpdateDescriptorSets(app->device.logical, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	}
}

void qb::Descriptor::destroy(){
	vkDestroyDescriptorSetLayout(app->device.logical, layout, nullptr);
}