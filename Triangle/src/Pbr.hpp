#pragma once
#include "engine/app.h"

struct MvpUniform {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 camPos;
};

struct LightUniform {
	glm::vec3 lightDir;
	float exposure = 4.5f;
	float gamma = 2.2f;
	float prefilteredCubeMipLevels = 1;
	float scaleIBLAmbient = 1.0f;
};

class Pbr : public qb::App {
	qb::Image* envCubeImg;
	qb::Image* brdfLutImg;
	qb::Image* irradianceCubeImg;
	qb::Image* prefilteredCubeImg;
	qb::Model* model;
	qb::GraphicsPipeline* pbrPipeline;
	qb::RenderPass* irradianceCubeRenderPass;
	qb::RenderPass* prefilteredCubeRenderPass;
	qb::RenderPass* brdfLutRenderPass;
	qb::FrameBuffer* cubeMapFrameBuffer;
	qb::FrameBuffer* brdfLutFrameBuffer;
	qb::GraphicsPipeline* irradianceCubePipeline;
	qb::GraphicsPipeline* prefilteredCubePipeline;
	qb::GraphicsPipeline* brdfLutPipeline;
	MvpUniform uniform;
	qb::Buffer* mvpUniBuf;
	qb::Descriptor* mvpDescriptor;
	qb::DescriptorDesc* mvpDescriptorDesc;
	qb::RenderPass* pbrRenderPass;
	qb::FrameBuffer* pbrFrameBuffer;
	qb::Buffer* lightUniBuf;
	qb::Descriptor* lightDescriptor;
	qb::DescriptorDesc* lightDescriptorDesc;
	glm::vec3 cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
	LightUniform lightUniform;
	virtual void onInit() {
		// model
		model = this->modelMgr.getModel("model");
		model->gltf = this->modelMgr.getGltf("models/gltfSample/DamagedHelmet/glTF/DamagedHelmet.gltf");
		model->build();
		// environment cube
		envCubeImg = this->bufferMgr.getImage("envCubeImg");
		envCubeImg->tex = this->bufferMgr.getTex("textures/cubemap.dds");
		envCubeImg->imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		envCubeImg->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		envCubeImg->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		envCubeImg->build();
		envCubeImg->setImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		irradianceCubeImg = this->bufferMgr.getImage("irridanceCubeImg");
		prefilteredCubeImg = this->bufferMgr.getImage("prefilteredCubeImg");
		brdfLutImg = this->bufferMgr.getImage("brdfLutImg");
		// irradiance cube / prefiltered cube
		//this->generateCubemaps();
		this->generateBrdfLut();
		// mvp uniform / descriptor set 0
		this->mvpUniBuf = this->bufferMgr.getBuffer("mvpUniBuf");
		this->mvpUniBuf->bufferInfo.size = sizeof(MvpUniform);
		this->mvpUniBuf->bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		this->mvpUniBuf->descriptorRange = sizeof(MvpUniform);
		this->mvpUniBuf->buildPerSwapchainImg();
		this->mvpDescriptorDesc = this->descriptorMgr.getDescriptorDesc("mvpDescriptorDesc");
		this->mvpDescriptorDesc->bindings = {
			descriptor_layout_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
		};
		this->mvpDescriptorDesc->build();
		this->mvpDescriptor = this->descriptorMgr.getDescriptor("mvpDescriptor");
		this->mvpDescriptor->desc = this->mvpDescriptorDesc;
		this->mvpDescriptor->datas = {
			mvpUniBuf
		};
		this->mvpDescriptor->buildPerSwapchainImg();
		// mesh uniform / descriptor set 1
		// material uniform / descriptor set 2
		// light uniform / descriptor set 3
		this->lightUniBuf = this->bufferMgr.getBuffer("lightUniBuf");
		this->lightUniBuf->bufferInfo.size = sizeof(LightUniform);
		this->lightUniBuf->bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		this->lightUniBuf->descriptorRange = sizeof(LightUniform);
		this->lightUniBuf->buildPerSwapchainImg();
		this->lightDescriptorDesc = this->descriptorMgr.getDescriptorDesc("lightDescritporDesc");
		this->lightDescriptorDesc->bindings = {
			descriptor_layout_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT),
			descriptor_layout_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
			descriptor_layout_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
			descriptor_layout_binding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
		};
		this->lightDescriptorDesc->build();
		this->lightDescriptor = this->descriptorMgr.getDescriptor("lightDescriptor");
		this->lightDescriptor->desc = this->lightDescriptorDesc;
		this->lightDescriptor->datas = {
			lightUniBuf,
			envCubeImg,
			envCubeImg,
			//irradianceCubeImg,
			//prefilteredCubeImg,
			brdfLutImg,
		};
		this->lightDescriptor->buildPerSwapchainImg();

		// pbr render pass
		pbrRenderPass = this->renderPassMgr.getRenderPass("pbrRenderPass");
		pbrRenderPass->build();
		//pbr frame buffer
		this->pbrFrameBuffer = this->bufferMgr.getFrameBuffer("pbrFrameBuffer");
		this->pbrFrameBuffer->renderPass = pbrRenderPass->renderPass;
		this->pbrFrameBuffer->onAttachments = [this](size_t i) -> std::vector<VkImageView> {
			return {
				this->swapchain.views[i],
				this->bufferMgr.defaultDepthImg->view,
			};
		};
		this->pbrFrameBuffer->build();

		// pipeline
		pbrPipeline = this->pipelineMgr.getGraphicsPipeline("pbrPipeline");
		pbrPipeline->renderPass = pbrRenderPass->renderPass;
		pbrPipeline->pipelineInfo.subpass = 0;
		pbrPipeline->shaderStages = {
			this->pipelineMgr.getShaderStage("shaders/pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			this->pipelineMgr.getShaderStage("shaders/pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
		};
		pbrPipeline->vertexInputInfo.vertexBindingDescriptionCount = 1;  // vertex binding
		pbrPipeline->vertexInputInfo.pVertexBindingDescriptions = &qb::Model::Vertex::getBindingDesc();
		auto attribDesc = qb::Model::Vertex::getAttribDesc();
		pbrPipeline->vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDesc.size());
		pbrPipeline->vertexInputInfo.pVertexAttributeDescriptions = attribDesc.data();

		std::array<VkDescriptorSetLayout, 4> descriptorSetLayouts = {
			mvpDescriptorDesc->layout,
			modelMgr.meshDescriptorDesc->layout,
			modelMgr.materialDescriptorDesc->layout,
			lightDescriptorDesc->layout,
		};
		pbrPipeline->pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pbrPipeline->pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

		pbrPipeline->depthStencil.depthTestEnable = VK_TRUE; // depth stencil binding
		pbrPipeline->build();
	}

	//void generateCubemaps() {
	//	std::vector<qb::Image*> imgs = { irradianceCubeImg, prefilteredCubeImg };
	//	enum Target {IRRADIANCE = 0, PREFILTERED = 1};
	//	for (uint32_t target = 0; target < PREFILTERED + 1; target++) {
	//		auto img = imgs[target];
	//		switch (target) {
	//		case IRRADIANCE:
	//			img->imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	//			img->imageInfo.extent = {64, 64, 1};
	//			break;
	//		case PREFILTERED:
	//			img->imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	//			img->imageInfo.extent = {512, 512, 1};
	//		}

	//		img->imageInfo.mipLevels = get_mips(img->imageInfo.extent.width);
	//		img->imageInfo.imageType = VK_IMAGE_TYPE_2D;
	//		img->imageInfo.arrayLayers = 6;
	//		img->imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//		img->imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	//		img->viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	//		img->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//		img->viewInfo.subresourceRange.levelCount = img->imageInfo.mipLevels;
	//		img->viewInfo.subresourceRange.layerCount = img->imageInfo.arrayLayers;
	//		img->samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	//		img->samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	//		img->samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	//		img->samplerInfo.maxLod = static_cast<float>(img->imageInfo.mipLevels);
	//		img->build();
	//		
	//		if (target == IRRADIANCE) {
	//			// render pass
	//			{
	//				this->irradianceCubeRenderPass = this->renderPassMgr.getRenderPass("irradianceCubeRenderPass");
	//				irradianceCubeRenderPass->attachmentDescs = {
	//					{0, img->imageInfo.format, VK_SAMPLE_COUNT_1_BIT,
	//					VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	//					}, // color attachment desc
	//				};
	//				irradianceCubeRenderPass->attachmentRefs = {
	//					{
	//						{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} // color attachment ref
	//					},	// sub pass 0
	//				};
	//				irradianceCubeRenderPass->subpassDescs = {
	//					{
	//						0, VK_PIPELINE_BIND_POINT_GRAPHICS,
	//						0 , nullptr, // input attachment ref
	//						1, &irradianceCubeRenderPass->attachmentRefs[0][0], // color attachment ref
	//						nullptr, // resolve attachment ref
	//						nullptr, // depth stencil attachment ref
	//						0, nullptr // preserve attachment ref
	//					}, // sub pass 0
	//				};
	//				irradianceCubeRenderPass->dependencies = {
	//					{
	//						VK_SUBPASS_EXTERNAL, 0,
	//						VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	//						VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	//						VK_DEPENDENCY_BY_REGION_BIT
	//					}, // external -> 0 mem -> color output
	//					{
	//						0, VK_SUBPASS_EXTERNAL,
	//						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
	//						VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
	//						VK_DEPENDENCY_BY_REGION_BIT
	//					}, // 0 -> external color output -> mem
	//				};
	//				irradianceCubeRenderPass->build();
	//			}
	//			// pipeline
	//			{
	//				//todo
	//			}
	//		}
	//	}
	//}

	void generateBrdfLut() {		
		uint32_t dim = 512;
		this->brdfLutImg->imageInfo.imageType = VK_IMAGE_TYPE_2D;
		this->brdfLutImg->imageInfo.format = VK_FORMAT_R16G16_SFLOAT;
		this->brdfLutImg->imageInfo.extent = {
			dim, dim, 1
		};
		this->brdfLutImg->imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		this->brdfLutImg->viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		this->brdfLutImg->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		this->brdfLutImg->samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		this->brdfLutImg->samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		this->brdfLutImg->samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		brdfLutImg->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		brdfLutImg->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		brdfLutImg->build();
		brdfLutImg->setImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		

		// render pass
		{
			auto& img = this->brdfLutImg;
			this->brdfLutRenderPass = this->renderPassMgr.getRenderPass("brdfLutRenderPass");
			brdfLutRenderPass->attachmentDescs = {
				{0, img->imageInfo.format, VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				}, // color attachment desc
			};
			brdfLutRenderPass->attachmentRefs = {
				{
					{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} // color attachment ref
				},	// sub pass 0
			};
			brdfLutRenderPass->subpassDescs = {
				{
					0, VK_PIPELINE_BIND_POINT_GRAPHICS,
					0 , nullptr, // input attachment ref
					1, &brdfLutRenderPass->attachmentRefs[0][0], // color attachment ref
					nullptr, // resolve attachment ref
					nullptr, // depth stencil attachment ref
					0, nullptr // preserve attachment ref
				}, // sub pass 0
			};
			brdfLutRenderPass->dependencies = {
				{
					VK_SUBPASS_EXTERNAL, 0,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_DEPENDENCY_BY_REGION_BIT
				}, // external -> 0 mem -> color output
				{
					0, VK_SUBPASS_EXTERNAL,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
					VK_DEPENDENCY_BY_REGION_BIT
				}, // 0 -> external color output -> mem
			};
			brdfLutRenderPass->build();
		}
		// pipeline
		{
			brdfLutPipeline = this->pipelineMgr.getGraphicsPipeline("pipeline");
			brdfLutPipeline->renderPass = brdfLutRenderPass->renderPass;
			brdfLutPipeline->pipelineInfo.subpass = 0;
			brdfLutPipeline->shaderStages = {
				this->pipelineMgr.getShaderStage("shaders/genBrdfLut.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
				this->pipelineMgr.getShaderStage("shaders/genBrdfLut.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
			};
			brdfLutPipeline->vertexInputInfo.vertexBindingDescriptionCount = 0;  // vertex binding
			brdfLutPipeline->vertexInputInfo.pVertexBindingDescriptions = nullptr;
			brdfLutPipeline->vertexInputInfo.vertexAttributeDescriptionCount = 0;
			brdfLutPipeline->vertexInputInfo.pVertexAttributeDescriptions = nullptr;

			brdfLutPipeline->depthStencil.depthTestEnable = VK_TRUE; // depth stencil binding
			brdfLutPipeline->build();
		}
		//frame buffer
		{
			brdfLutFrameBuffer = this->bufferMgr.getFrameBuffer("brdfLutFrameBuffer");
			brdfLutFrameBuffer->renderPass = brdfLutRenderPass->renderPass;
			brdfLutFrameBuffer->createInfo.width = dim;
			brdfLutFrameBuffer->createInfo.height = dim;
			brdfLutFrameBuffer->onAttachments = [this](size_t i) -> std::vector<VkImageView> {
				return {
					this->brdfLutImg->view,
				};
			};
			brdfLutFrameBuffer->build();
		}
		// cmd buf
		{
			auto cmdBuf = this->bufferMgr.beginOnce();
			this->brdfLutRenderPass->clearValues = {
				{0.0f, 0.0f, 0.0f, 1.0f}, // color
			};
			this->brdfLutRenderPass->framebuffer = this->brdfLutFrameBuffer;
			this->brdfLutRenderPass->begin(0, &cmdBuf);
			vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, brdfLutPipeline->pipeline);
			vkCmdDraw(cmdBuf, 3, 1, 0, 0);
			this->brdfLutRenderPass->end(0, &cmdBuf);
			this->bufferMgr.endOnce(&cmdBuf);
		}
		// img layout
		{
			//brdfLutImg->setImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}
	}
	
	void renderNode(qb::Model::Node* node, uint32_t i, qb::Model::Material::AlphaMode alphaMode) {
		if (node->mesh) {
			for (auto& primitive : node->mesh->primitives) {
				if (primitive->material->alphaMode == alphaMode) {
					const std::vector<VkDescriptorSet> descriptors = {
						mvpDescriptor->sets(i),
						node->mesh->descriptor->sets(i),
						primitive->descriptor->sets(i),
						lightDescriptor->sets(i)
					};
					vkCmdBindDescriptorSets(this->bufferMgr.cmdBuf(i), VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipeline->layout, 0, static_cast<uint32_t>(descriptors.size()), descriptors.data(), 0, nullptr);
					vkCmdDrawIndexed(this->bufferMgr.cmdBuf(i), primitive->indexCount, 1, primitive->firstIndex, 0, 0);
				}
			}
		}
		for (auto& child : node->children) {
			renderNode(child, i, alphaMode);
		}
	}

	virtual void onCmd(size_t i) {
		this->bufferMgr.begin(i);
		this->pbrRenderPass->clearValues = {
			{0.0f, 0.0f, 0.0f, 1.0f}, // swap chain
			{1.0f, 0}, // depth stencil
		};
		this->pbrRenderPass->framebuffer = this->pbrFrameBuffer;
		this->pbrRenderPass->begin(i);
		
		vkCmdBindPipeline(this->bufferMgr.cmdBuf(i), VK_PIPELINE_BIND_POINT_GRAPHICS, this->pbrPipeline->pipeline);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(this->bufferMgr.cmdBuf(i), qb::Model::Vertex::getBinding(), 1, &model->vertexBuf->buffer(), &offset);
		vkCmdBindIndexBuffer(this->bufferMgr.cmdBuf(i), model->indexBuf->buffer(), 0, VK_INDEX_TYPE_UINT32);
		renderNode(model->rootNode, (uint32_t)i, qb::Model::Material::ALPHAMODE_OPAQUE);
		this->pbrRenderPass->end(i);
		this->bufferMgr.end(i);
	}

	virtual void onLoop() {
		// uniform update
		float time = this->timerMgr.getTotalTime();

		// mvp uniform
		uniform.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
		uniform.view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		uniform.proj = glm::perspective(glm::radians(45.0f), (float)this->swapchain.extent.width / (float)this->swapchain.extent.width, 0.1f, 10.0f);
		uniform.camPos = cameraPos;
		this->mvpUniBuf->mappingCurSwapchainImg(&uniform);

		// node/materail uniform
		model->updateAnimation(time);

		// light param
		lightUniform.lightDir = cameraPos - glm::vec3(0.0f, 0.0f, 0.0f);
		lightUniform.prefilteredCubeMipLevels = (float)this->envCubeImg->imageInfo.mipLevels;
		this->lightUniBuf->mappingCurSwapchainImg(&lightUniform);
	}

	virtual void onDestroy() {

	}
};