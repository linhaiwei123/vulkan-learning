#include "../engine/app.h"
struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	vertex_binding_desc(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
	vertex_attrib_desc(
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) },
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) },
		{2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)}
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
	qb::Image* img;
	qb::Image* depthImg;
	PushConst pushConst;
	VkPushConstantRange pushConstRange;
	virtual void onInit() {
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
		pushConstRange = { VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConst) };
		// image
		img = this->bufferMgr.getImage("img");
		img->tex = this->bufferMgr.getTex("./textures/texture.dds");
		img->build();
		// descriptor
		descriptor = this->descriptorMgr.getDescriptor("descriptor");
		descriptor->bindings = {
			{descriptor_layout_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT), uniBuf},
			{img->getLayoutBinding(1, VK_SHADER_STAGE_FRAGMENT_BIT), img}
		};
		descriptor->build();
		// depth
		depthImg = this->bufferMgr.getImage("depthImg");
		depthImg->buildDepth();
		// render pass
		renderPass = this->renderPassMgr.getRenderPass("renderPass");
		renderPass->attachmentDescs = {
			{0, swapchain.format, VK_SAMPLE_COUNT_1_BIT, 
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, // load / store op
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencil load / store op
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // initial  finish layout
			}, // swapchain attachment desc
			{0, device.depthFormat, VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, 
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			} // depth attachment desc
		};
		renderPass->attachmentRefs = {
			{
				{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, // swap chain attachment ref
				{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}, // depth stencil attachment ref
			} // sub pass 1
		};
		renderPass->subpassDescs = {
			{0, VK_PIPELINE_BIND_POINT_GRAPHICS, 
			0, nullptr, // input attachment ref
			1, &renderPass->attachmentRefs[0][0], // color attachment ref
			nullptr,  // resolve attachment ref
			&renderPass->attachmentRefs[0][1], // depth stencil attachment ref
			0, nullptr // preserve attachment ref
			} // sub pass 1 
		};
		renderPass->build();
		// pipeline
		pipeline = this->pipelineMgr.getGraphicsPipeline("pipeline");
		pipeline->renderPass = renderPass->renderPass;
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
		pipeline->pipelineLayoutInfo.pPushConstantRanges = &pushConstRange;

		pipeline->depthStencil.depthTestEnable = VK_TRUE; // depth stencil binding
		pipeline->build();
		// frame buffer
		this->bufferMgr.renderPass = renderPass->renderPass;
		this->bufferMgr.onAttachments = [this](size_t i) -> std::vector<VkImageView> {
			return {
				this->swapchain.views[i],
				depthImg->view 
			};
		};
		this->bufferMgr.build();
	};

	virtual void onCmd(size_t i) {
		// command buffer
		this->bufferMgr.begin(i);
		this->renderPass->clearValues = {
			{0.0f, 0.0f, 0.0f, 1.0f}, // color 
			{1.0f, 0} // depth stencil
		};
		this->renderPass->begin(i);
		vkCmdBindPipeline(this->bufferMgr.cmdBuf(i), VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->pipeline);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(this->bufferMgr.cmdBuf(i), 0, 1, &vertexBuf->buffer(), &offset);
		vkCmdBindIndexBuffer(this->bufferMgr.cmdBuf(i), indexBuf->buffer(), 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(this->bufferMgr.cmdBuf(i), VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline->layout, 0, 1, 
			&this->descriptor->sets(i), 0, nullptr);
		vkCmdDrawIndexed(this->bufferMgr.cmdBuf(i), static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		this->renderPass->end(i);
		this->bufferMgr.end(i);
	}

	virtual void onLoop() {
		// uniform update
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		uniform.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		uniform.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		uniform.proj = glm::perspective(glm::radians(45.0f), (float)this->swapchain.extent.width / (float)this->swapchain.extent.width, 
			0.1f, 10.0f);
		uniform.proj[1][1] *= -1;
		this->uniBuf->mappingCurSwapchainImg(&uniform);

		// push const update
		auto cmdBuf = this->bufferMgr.beginOnce();
		pushConst.speed = glm::vec2(0.5f, 0.5f) * time;
		vkCmdPushConstants(cmdBuf, this->pipeline->layout, pushConstRange.stageFlags, pushConstRange.offset, pushConstRange.size, &pushConst);
		this->bufferMgr.endOnce(cmdBuf);
	};

	virtual void onDestroy() {

	};
};