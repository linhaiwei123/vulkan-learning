#include "../engine/app.h"
struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	vertex_desc(Vertex, 0, VK_VERTEX_INPUT_RATE_VERTEX,
		{ 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) },
		{ 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) },
		{ 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord) }
	);
};

struct Uniform {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct PushConst {
	glm::vec2 speed;
};

class Triangle : public qb::App {
	qb::RenderPass* renderPass;
	qb::GraphicsPipeline* pipeline;
	std::vector<Vertex> vertices;
	qb::Buffer* vertexBuf;
	std::vector<uint16_t> indices;
	qb::Buffer* indexBuf;
	Uniform uniform;
	qb::Buffer* uniBuf;
	qb::Descriptor* descriptor;
	qb::Image* tex2dImg;
	qb::Image* depthImg;
	PushConst pushConst;
	VkPushConstantRange graphicsPushConstRange;
	VkPushConstantRange computePushConstRange;
	qb::Image* colorImg;
	qb::GraphicsPipeline* writePipeline;
	qb::Image* storeImg;
	qb::Descriptor* compDescriptor;
	qb::ComputePipeline* compPipeline;
	qb::Image* tex2dArrayImg;
	qb::Model* model;
	virtual void onInit() {
		//model
		model = this->modelMgr.getModel("model");
		model->gltf = this->modelMgr.getGltf("./models/cube.gltf");
		model->build();
		model->setAnimation("Action");
		// vertex
		std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		};
		vertexBuf = this->bufferMgr.getBuffer("vertexBuf");
		vertexBuf->bufferInfo.size = sizeof(Vertex) * vertices.size();
		vertexBuf->bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; 
		vertexBuf->build();
		vertexBuf->mapping(vertices.data());
		// index
		indices = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};
		indexBuf = this->bufferMgr.getBuffer("indexBuf");
		indexBuf->bufferInfo.size = sizeof(uint16_t) * indices.size();
		indexBuf->bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexBuf->build();
		indexBuf->mapping(indices.data());
		// uniform
		uniform = {};
		uniBuf = this->bufferMgr.getBuffer("uniBuf");
		uniBuf->bufferInfo.size = sizeof(Uniform);
		uniBuf->bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		uniBuf->descriptorRange = sizeof(Uniform);
		uniBuf->buildPerSwapchainImg();
		// push const
		graphicsPushConstRange = { VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConst) };
		computePushConstRange = { VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConst) };
		// tex 2d image
		tex2dImg = this->bufferMgr.getImage("tex2dImg");
		tex2dImg->tex = this->bufferMgr.getTex("./textures/tex2d.dds");
		tex2dImg->imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		tex2dImg->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		tex2dImg->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		tex2dImg->build();
		tex2dImg->setImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		// tex 2d array image
		tex2dArrayImg = this->bufferMgr.getImage("tex2dArrayImg");
		tex2dArrayImg->tex = this->bufferMgr.getTex("./textures/tex2dArray.dds");
		tex2dArrayImg->imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		tex2dArrayImg->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		tex2dArrayImg->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		tex2dArrayImg->build();
		tex2dArrayImg->setImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		// depth attachment
		depthImg = this->bufferMgr.getImage("depthImg");
		depthImg->imageInfo.imageType = VK_IMAGE_TYPE_2D;
		depthImg->imageInfo.format = this->device.depthFormat;
		depthImg->imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depthImg->viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthImg->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		depthImg->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthImg->build();
		depthImg->setImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
		// color attachment
		colorImg = this->bufferMgr.getImage("colorImg");
		colorImg->imageInfo.imageType = VK_IMAGE_TYPE_2D;
		colorImg->imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		colorImg->imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		colorImg->viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorImg->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorImg->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colorImg->build();
		colorImg->setImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		// store image
		storeImg = this->bufferMgr.getImage("storeImg");
		storeImg->imageInfo.imageType = VK_IMAGE_TYPE_2D;
		storeImg->imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		storeImg->imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		storeImg->viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		storeImg->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		storeImg->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		storeImg->build();
		storeImg->setImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		// descriptor
		descriptor = this->descriptorMgr.getDescriptor("descriptor");
		descriptor->bindings = {
			{descriptor_layout_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT), uniBuf},
			{descriptor_layout_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT), tex2dImg},
			{descriptor_layout_binding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT), colorImg},
			{descriptor_layout_binding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT), storeImg},
			{descriptor_layout_binding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT), tex2dArrayImg},
		};
		descriptor->buildPerSwapchainImg();
		// compute descriptor
		compDescriptor = this->descriptorMgr.getDescriptor("computeDescriptor");
		compDescriptor->bindings = {
			{descriptor_layout_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT), storeImg},
		};
		compDescriptor->build();
		// render pass
		renderPass = this->renderPassMgr.getRenderPass("renderPass");
		renderPass->attachmentDescs = {
			{0, swapchain.format, VK_SAMPLE_COUNT_1_BIT, 
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, // load / store op
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencil load / store op
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // initial  finish layout
			}, // swapchain attachment desc
			{0, depthImg->imageInfo.format, VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, 
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			}, // depth attachment desc
			{0, colorImg->imageInfo.format, VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			}, // color attachment desc
		};
		renderPass->attachmentRefs = {
			{
				{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} // color attachment ref
			},	// sub pass 0
			{
				{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, // swap chain attachment ref
				{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}, // depth stencil attachment ref
				{2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL } // color attachment ref -> input attachment ref
			} // sub pass 1
		};
		renderPass->subpassDescs = {
			{
				0, VK_PIPELINE_BIND_POINT_GRAPHICS,
				0 , nullptr, // input attachment ref
				1, &renderPass->attachmentRefs[0][0], // color attachment ref
				nullptr, // resolve attachment ref
				nullptr, // depth stencil attachment ref
				0, nullptr // preserve attachment ref
			}, // sub pass 0
			{
				0, VK_PIPELINE_BIND_POINT_GRAPHICS, 
				1, &renderPass->attachmentRefs[1][2], // input attachment ref
				1, &renderPass->attachmentRefs[1][0], // color attachment ref
				nullptr,  // resolve attachment ref
				&renderPass->attachmentRefs[1][1], // depth stencil attachment ref
				0, nullptr // preserve attachment ref
			} // sub pass 1
		};
		renderPass->dependencies = {
			{
				VK_SUBPASS_EXTERNAL, 0, 
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_DEPENDENCY_BY_REGION_BIT
			}, // external -> 0 mem -> color output
			{
				0, 1,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_DEPENDENCY_BY_REGION_BIT
			}, // 0 -> 1 color output -> input
			{
				0, VK_SUBPASS_EXTERNAL,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
				VK_DEPENDENCY_BY_REGION_BIT
			}, // 0 -> external color output -> mem
		};
		renderPass->build();
		// write pipeline
		{
			writePipeline = this->pipelineMgr.getGraphicsPipeline("writePipeline");
			writePipeline->renderPass = renderPass->renderPass;
			writePipeline->pipelineInfo.subpass = 0;
			writePipeline->shaderStages = {
				this->pipelineMgr.getShaderStage("./shaders/writePipeline.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
				this->pipelineMgr.getShaderStage("./shaders/writePipeline.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
			};
			writePipeline->vertexInputInfo.vertexBindingDescriptionCount = 1;  // vertex binding
			writePipeline->vertexInputInfo.pVertexBindingDescriptions = &qb::Model::Vertex::getBindingDesc();
			auto attribDesc = qb::Model::Vertex::getAttribDesc();
			writePipeline->vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDesc.size());
			writePipeline->vertexInputInfo.pVertexAttributeDescriptions = attribDesc.data();

			writePipeline->pipelineLayoutInfo.setLayoutCount = 1;
			writePipeline->pipelineLayoutInfo.pSetLayouts = &model->linearMeshes[0]->descriptor->layout;

			writePipeline->build();
		}
		// compute pipeline
		{
			compPipeline = this->pipelineMgr.getComputePipeline("computePipeline");
			compPipeline->shaderStage = this->pipelineMgr.getShaderStage("./shaders/compute.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
			compPipeline->pipelineLayoutInfo.setLayoutCount = 1;
			compPipeline->pipelineLayoutInfo.pSetLayouts = &compDescriptor->layout;
			compPipeline->pipelineLayoutInfo.pushConstantRangeCount = 1;
			compPipeline->pipelineLayoutInfo.pPushConstantRanges = &computePushConstRange;
			compPipeline->build();
		}
		// pipeline
		{
			pipeline = this->pipelineMgr.getGraphicsPipeline("pipeline");
			pipeline->renderPass = renderPass->renderPass;
			pipeline->pipelineInfo.subpass = 1;
			pipeline->shaderStages = {
				this->pipelineMgr.getShaderStage("./shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
				this->pipelineMgr.getShaderStage("./shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
			};
			pipeline->vertexInputInfo.vertexBindingDescriptionCount = 1;  // vertex binding
			pipeline->vertexInputInfo.pVertexBindingDescriptions = &Vertex::getBindingDesc();
			auto attribDesc = Vertex::getAttribDesc();
			pipeline->vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDesc.size());
			pipeline->vertexInputInfo.pVertexAttributeDescriptions = attribDesc.data();

			pipeline->pipelineLayoutInfo.setLayoutCount = 1; // uniform binding
			pipeline->pipelineLayoutInfo.pSetLayouts = &this->descriptor->layout;

			pipeline->pipelineLayoutInfo.pushConstantRangeCount = 1; // push const range binding
			pipeline->pipelineLayoutInfo.pPushConstantRanges = &graphicsPushConstRange;

			pipeline->depthStencil.depthTestEnable = VK_TRUE; // depth stencil binding
			pipeline->build();
		}
		// frame buffer
		this->bufferMgr.renderPass = renderPass->renderPass;
		this->bufferMgr.onAttachments = [this](size_t i) -> std::vector<VkImageView> {
			return {
				this->swapchain.views[i],
				depthImg->view,
				colorImg->view,
			};
		};
		this->bufferMgr.build();
	};

	virtual void onCmd(size_t i) {
		// command buffer
		this->bufferMgr.begin(i);

		storeImg->setImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, this->bufferMgr.cmdBuf(i));
		this->renderPass->clearValues = {
			{0.0f, 0.0f, 0.0f, 1.0f}, // swap chain
			{1.0f, 0}, // depth stencil
			{0.0f, 0.0f, 0.0f, 1.0f} // color
		};
		this->renderPass->begin(i);
		// sub pass 0
		{
			vkCmdBindPipeline(this->bufferMgr.cmdBuf(i), VK_PIPELINE_BIND_POINT_GRAPHICS, this->writePipeline->pipeline);
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(this->bufferMgr.cmdBuf(i), qb::Model::Vertex::getBinding(), 1, &model->vertexBuf->buffer(), &offset);
			vkCmdBindIndexBuffer(this->bufferMgr.cmdBuf(i), model->indexBuf->buffer(), 0, VK_INDEX_TYPE_UINT32);
			for (auto& mesh : model->linearMeshes) {
				vkCmdBindDescriptorSets(this->bufferMgr.cmdBuf(i), VK_PIPELINE_BIND_POINT_GRAPHICS, this->writePipeline->layout, 0, 1, &mesh->descriptor->sets(i), 0, nullptr);
				for (auto& primitive : mesh->primitives) {
					vkCmdDrawIndexed(this->bufferMgr.cmdBuf(i), primitive->indexCount, 1, primitive->firstIndex, 0, 0);
				}
			}
		}
		// sub pass 1
		{
			vkCmdNextSubpass(this->bufferMgr.cmdBuf(i), VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(this->bufferMgr.cmdBuf(i), VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->pipeline);
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(this->bufferMgr.cmdBuf(i), Vertex::getBinding(), 1, &vertexBuf->buffer(), &offset);
			vkCmdBindIndexBuffer(this->bufferMgr.cmdBuf(i), indexBuf->buffer(), 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(this->bufferMgr.cmdBuf(i), VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->layout, 0, 1,
				&this->descriptor->sets(i), 0, nullptr);
			vkCmdDrawIndexed(this->bufferMgr.cmdBuf(i), static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		}
		this->renderPass->end(i);
		this->bufferMgr.end(i);
	}

	virtual void onLoop() {
		// uniform update
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		uniform.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));
		uniform.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		uniform.proj = glm::perspective(glm::radians(45.0f), (float)this->swapchain.extent.width / (float)this->swapchain.extent.width, 0.1f, 10.0f);
		this->uniBuf->mappingCurSwapchainImg(&uniform);

		// mode update
		glm::mat4 modelM = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 modelV = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		glm::mat4 modelP = glm::perspective(glm::radians(45.0f), (float)this->swapchain.extent.width / (float)this->swapchain.extent.width, 0.1f, 10.0f);
		model->rootNode->mat = modelP * modelV * modelM;
		model->updateAnimation(time);

		// push const update
		auto cmdBuf = this->bufferMgr.beginOnce();
		pushConst.speed = glm::vec2(0.5f, 0.5f) * time;
		vkCmdPushConstants(cmdBuf, this->pipeline->layout, graphicsPushConstRange.stageFlags, graphicsPushConstRange.offset, graphicsPushConstRange.size, &pushConst);
		vkCmdPushConstants(cmdBuf, this->compPipeline->layout, computePushConstRange.stageFlags, computePushConstRange.offset, computePushConstRange.size, &pushConst);
		vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, compPipeline->pipeline);
		vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, compPipeline->layout, 0, 1, &compDescriptor->sets(), 0, 0);
		vkCmdDispatch(cmdBuf, storeImg->imageInfo.extent.width / 16, storeImg->imageInfo.extent.height / 16, 1);
		this->bufferMgr.endOnce(cmdBuf);
	};

	virtual void onDestroy() {

	};
};