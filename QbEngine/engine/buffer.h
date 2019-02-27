#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
#include <optional>
#include <unordered_map>
#include <gli/gli.hpp>

namespace qb {
	class App;
	class Buffer;
	class Image;
	class BufferMgr {
	private:
		std::unordered_map<std::string, qb::Buffer*> _bufferMap{};
		std::unordered_map<std::string, qb::Image*> _imageMap{};
		std::unordered_map <std::string, gli::texture*> _texMap{}; 
	public:
		App *app;
		std::vector<VkFramebuffer> framebuffers;
		std::vector<VkCommandBuffer> commandBuffers;
		VkRenderPassCreateInfo renderPassCreateInfo{};
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		std::function<std::vector<VkImageView>(size_t i)> onAttachments = nullptr;
	public:
		BufferMgr() = default;

		void init(App *app);

		void build();

		void begin(size_t i);

		void end(size_t i);

		VkCommandBuffer beginOnce();

		void endOnce(VkCommandBuffer cmdBuf);

		void destroy();

		Buffer* getBuffer(std::string name);
		void destroyBuffer(std::string name);

		Image* getImage(std::string name);

		inline VkCommandBuffer cmdBuf(size_t i) { return commandBuffers[i]; };

		gli::texture* getTex(std::string name);
	};

	class Buffer {
	private:
		App* app;
		std::string name;
	public:
		VkBufferCreateInfo bufferInfo{};
		std::vector<VkBuffer> buffers{};
		VkDeviceSize descriptorRange = 0;
		std::vector<VkDescriptorBufferInfo> descriptorBufferInfos{};
		std::vector<VkDeviceMemory> mems{};
	public:
		void init(App* app, std::string name);
		
		void build(size_t count=1);
		void buildPerSwapchainImg();
		
		void mapping(void* data, size_t i=0, size_t offset=0, size_t size=0);

		void mappingCurSwapchainImg(void* data, size_t offset=0, size_t size=0);
		
		void destroy();

		inline VkBuffer& buffer(int i = 0) { return buffers[i]; };
		inline VkDeviceMemory& mem(int i = 0) { return mems[i]; };

		void copyToImage(qb::Image* img);
	};

	class Image {
	private:
		App* app;
		std::string name;
		qb::Buffer* stageBuffer;
		VkImageLayout _imgLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	public:
		size_t texelAlign = 0;
		VkImageCreateInfo imageInfo{};
		VkImageViewCreateInfo viewInfo{};
		VkSamplerCreateInfo samplerInfo{};
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		VkDeviceMemory mem = VK_NULL_HANDLE;
		gli::texture* tex = nullptr;
		VkDescriptorImageInfo descriptorImageInfo = {};
	private:
		VkImageType _getImageTypeFromTex();
		VkImageViewType _getImageViewTypeFromTex();
		VkComponentMapping _getImageViewComponentMappingFromTex();
		size_t _getTexelAlign();
	public:
		void init(App* app, std::string name);

		void build();

		void mapping(void* data, size_t size=0);

		void destroy();

		void setImageLayout(VkImageLayout layout, VkPipelineStageFlagBits srcStage, VkPipelineStageFlagBits dstStage, VkCommandBuffer cmdBuf=VK_NULL_HANDLE);

		VkImageLayout getImageLayout();
	};
};