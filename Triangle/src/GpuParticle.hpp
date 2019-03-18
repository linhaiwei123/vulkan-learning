#pragma once
#include "engine/app.h"
#include "glm/gtc/random.hpp"

#define PARTICLE_COUNT 256 * 1024

class GpuParticle : public qb::App {
	struct Img {
		qb::Image* particle;
		qb::Image* gradient;
	} imgs;
	struct Vertex {
		struct Particle {
			glm::vec2 pos;
			glm::vec2 velocity;
			glm::vec4 gradient;
			vertex_desc(Particle, 0, VK_VERTEX_INPUT_RATE_VERTEX,
				{ 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Particle, pos) },
				//{ 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Particle, velocity) },
				{ 2, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Particle, gradient) },
			);
		};
		std::vector<Vertex::Particle> datas;
	} vertices;
	struct GraphicsPipe {
		qb::DescriptorDesc* desc;
		qb::Descriptor* descriptor;
		qb::GraphicsPipeline* pipeline;
		qb::RenderPass* renderPass;
		qb::FrameBuffer* frameBuffer;
	} graphics;
	struct ComputePipe {
		qb::Buffer* storageBuffer;
		qb::Buffer* uniformBuffer;
		qb::DescriptorDesc* desc;
		qb::Descriptor* descriptor;
		qb::ComputePipeline* pipeline;
		struct ComputeUbo {
			float dt;
			float x;
			float y;
			int32_t particleCount = PARTICLE_COUNT;
		} ubo;
	} compute;

	glm::vec2 pos{};
	
	virtual void onInit() {
		// mouse to move particle
		inputMgr.eventEmitter->on("mouse-move", [this](qb::Event& event) {
			auto& detail = std::any_cast<qb::InputMgr::EventDetail::MouseMove&>(event.detail);
			pos.x = (static_cast<float>(detail.x) - static_cast<float>(this->swapchain.extent.width / 2)) / static_cast<float>(this->swapchain.extent.width / 2);
			pos.y = (static_cast<float>(detail.y) - static_cast<float>(this->swapchain.extent.height / 2)) / static_cast<float>(this->swapchain.extent.height / 2);
		});

		// storage buffer (as vertex buffer)	
		vertices.datas.resize(PARTICLE_COUNT, {});
		for (auto& particle : vertices.datas) {
			particle.pos = glm::vec2(0.0f);
			particle.velocity = glm::diskRand(1.0f);
			particle.gradient = glm::vec4(1.0, 0.0, 0.0, 0.0);
		}
		compute.storageBuffer = this->bufferMgr.getBuffer("compute storage buffer");
		compute.storageBuffer->bufferInfo.size = sizeof(Vertex::Particle) * PARTICLE_COUNT;
		compute.storageBuffer->bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		compute.storageBuffer->descriptorRange = sizeof(Vertex::Particle) * PARTICLE_COUNT;
		compute.storageBuffer->build(1);
		compute.storageBuffer->mapping(vertices.datas.data());

		// uniform buffer
		compute.uniformBuffer = this->bufferMgr.getBuffer("compute uniform buffer");
		compute.uniformBuffer->bufferInfo.size = sizeof(ComputePipe::ComputeUbo);
		compute.uniformBuffer->bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		compute.uniformBuffer->descriptorRange = sizeof(ComputePipe::ComputeUbo);
		compute.uniformBuffer->build(1);

		// compute descriptor
		compute.desc = this->descriptorMgr.getDescriptorDesc("compute desc");
		compute.desc->bindings = {
			descriptor_layout_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT),
			descriptor_layout_binding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		};
		compute.desc->build();

		compute.descriptor = this->descriptorMgr.getDescriptor("compute descriptor");
		compute.descriptor->desc = compute.desc;
		compute.descriptor->datas = {
			compute.storageBuffer,
			compute.uniformBuffer
		};
		compute.descriptor->build(1);
		
		// compute pipeline
		compute.pipeline = this->pipelineMgr.getComputePipeline("compute pipeline");
		compute.pipeline->shaderStage = this->pipelineMgr.getShaderStage("shaders/particle.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
		compute.pipeline->pipelineLayoutInfo.setLayoutCount = 1;
		compute.pipeline->pipelineLayoutInfo.pSetLayouts = &compute.desc->layout;
		compute.pipeline->build();

		// imgs
		imgs.particle = this->bufferMgr.getImage("particle img");
		imgs.particle->tex = this->bufferMgr.getTex("textures/particle_bulb.dds");
		imgs.particle->imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		imgs.particle->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imgs.particle->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgs.particle->build();
		imgs.particle->setImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		imgs.gradient = this->bufferMgr.getImage("gradient img");
		imgs.gradient->tex = this->bufferMgr.getTex("textures/particle_gradient.ktx");
		imgs.gradient->imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		imgs.gradient->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imgs.gradient->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgs.gradient->build();
		imgs.gradient->setImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		// graphics render pass
		graphics.renderPass = this->renderPassMgr.getRenderPass("graphics render pass");
		graphics.renderPass->build();

		// graphics frame buffer
		graphics.frameBuffer = this->bufferMgr.getFrameBuffer("graphics frame buffer");
		graphics.frameBuffer->renderPass = graphics.renderPass->renderPass;
		graphics.frameBuffer->onAttachments = [this](size_t i) -> std::vector<VkImageView> {
			return {
				this->swapchain.views[i],
				this->bufferMgr.defaultDepthImg->view,
			};
		};
		graphics.frameBuffer->build();

		// graphics desc
		graphics.desc = this->descriptorMgr.getDescriptorDesc("graphics desc");
		graphics.desc->bindings = {
			descriptor_layout_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
			descriptor_layout_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
		};
		graphics.desc->build();

		graphics.descriptor = this->descriptorMgr.getDescriptor("graphics descriptor");
		graphics.descriptor->desc = graphics.desc;
		graphics.descriptor->datas = {
			imgs.particle,
			imgs.gradient
		};
		graphics.descriptor->buildPerSwapchainImg();

		// graphics pipeline
		graphics.pipeline = this->pipelineMgr.getGraphicsPipeline("graphics pipeline");
		graphics.pipeline->renderPass = graphics.renderPass->renderPass;
		graphics.pipeline->pipelineInfo.subpass = 0;
		graphics.pipeline->shaderStages = {
			this->pipelineMgr.getShaderStage("shaders/particle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			this->pipelineMgr.getShaderStage("shaders/particle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
		};
		graphics.pipeline->vertexInputInfo.vertexBindingDescriptionCount = 1;  // vertex binding
		graphics.pipeline->vertexInputInfo.pVertexBindingDescriptions = &Vertex::Particle::getBindingDesc();
		auto attribDesc = Vertex::Particle::getAttribDesc();
		graphics.pipeline->vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDesc.size());
		graphics.pipeline->vertexInputInfo.pVertexAttributeDescriptions = attribDesc.data();

		graphics.pipeline->pipelineLayoutInfo.setLayoutCount = 1;
		graphics.pipeline->pipelineLayoutInfo.pSetLayouts = &graphics.desc->layout;

		graphics.pipeline->inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

		graphics.pipeline->build();
	};
	virtual void onCmd(size_t i) {
		this->bufferMgr.begin(i);
		graphics.renderPass->clearValues = {
			{0.1f, 0.1f, 0.1f, 1.0f}, // swap chain
			{1.0f, 0}, // depth stencil
		};
		graphics.renderPass->framebuffer = graphics.frameBuffer;
		graphics.renderPass->begin(i);
		vkCmdBindPipeline(this->bufferMgr.cmdBuf(i), VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline->pipeline);
		vkCmdBindDescriptorSets(this->bufferMgr.cmdBuf(i), VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline->layout, 0, 1, &graphics.descriptor->sets(i), 0, nullptr);
		VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(this->bufferMgr.cmdBuf(i), 0, 1, &compute.storageBuffer->buffer(), offsets);
		vkCmdDraw(this->bufferMgr.cmdBuf(i), PARTICLE_COUNT, 1, 0, 0);
		graphics.renderPass->end(i);
		this->bufferMgr.end(i);
	};
	virtual void onLoop() {
		compute.ubo.x = pos.x;
		compute.ubo.y = pos.y;
		compute.ubo.dt = this->timerMgr.getDeltaTime();
		compute.uniformBuffer->mapping(&compute.ubo, 0);

		auto cmdBuf = this->bufferMgr.beginOnce();
		/*VkBufferMemoryBarrier bufferBarrier = {};
		bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier.buffer = compute.storageBuffer->buffer();
		bufferBarrier.size = compute.storageBuffer->bufferInfo.size;
		bufferBarrier.srcAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		bufferBarrier.srcQueueFamilyIndex = this->device.queueFamilyIndices.graphics.value();
		bufferBarrier.dstQueueFamilyIndex = this->device.queueFamilyIndices.compute.value();
		vkCmdPipelineBarrier(
			cmdBuf,
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0, nullptr,
			1, &bufferBarrier,
			0, nullptr);*/
		vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipeline->pipeline);
		vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipeline->layout, 0, 1, &compute.descriptor->sets(), 0, nullptr);
		vkCmdDispatch(cmdBuf, PARTICLE_COUNT / 256, 1, 1);
		/*bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;			
		bufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;	
		vkCmdPipelineBarrier(
			cmdBuf,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			0,
			0, nullptr,
			1, &bufferBarrier,
			0, nullptr);*/
		this->bufferMgr.endOnce(&cmdBuf);
	};
	virtual void onDestroy() {
		
	};
};