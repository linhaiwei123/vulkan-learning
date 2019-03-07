#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
#include <any>
#include <unordered_map>
namespace qb {
	class App;
	class DescriptorDesc;
	class Descriptor;
	class Buffer;
	class DescriptorMgr {
	private:
		std::unordered_map<std::string, qb::Descriptor*>  _descriptorMap;
		std::unordered_map<std::string, qb::DescriptorDesc*>  _descriptorDescMap;
	public:
		App *app;
		VkDescriptorPool descriptorPool;
	public:
		DescriptorMgr() = default;

		void init(App *app);

		Descriptor* getDescriptor(std::string name);

		DescriptorDesc* getDescriptorDesc(std::string name);

		void destroy();

		void update();
	};

	class Descriptor {	
	public:
		App* app;
		std::string name;
		std::vector<VkDescriptorSet> descriptorSets;
		size_t _descriptorSetDirtyNum = 0;
	public:
		std::vector<void*> datas{};
		qb::DescriptorDesc* desc;
	public:
		static bool is_descriptor_type_uniform(VkDescriptorType type);
		static bool is_descriptor_type_texel_buffer(VkDescriptorType type);
		static bool is_descriptor_type_image(VkDescriptorType type);
		Descriptor() = default;
		void init(App* app, std::string name);

		void build(size_t count = 1);

		void buildPerSwapchainImg();

		void setDescriptorSetDirty();

		void update();

		inline VkDescriptorSet& sets(size_t i = 0) { return descriptorSets[i]; };
		void destroy();
	};

	class DescriptorDesc {
	public: // args
		std::vector<VkDescriptorSetLayoutBinding> bindings{};
	public: // result
		VkDescriptorSetLayoutCreateInfo layoutInfo;
		VkDescriptorSetLayout layout;
	public:
		App* app;
		std::string name;
	public:
		DescriptorDesc() = default;
		void init(App* app, std::string name);
		void build();
		void destroy();
	};
};