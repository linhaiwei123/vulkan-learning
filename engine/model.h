#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <assert.h>
#include "tool.h"
#include <unordered_map>
#include <array>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define STBI_MSC_SECURE_CRT
#include <tiny_gltf.h>

namespace qb {
	class App;
	class Buffer;
	class Descriptor;
	class Image;
	class Model;
	class ModelMgr {
	private:
		std::unordered_map<std::string, qb::Model*> _modelMap{};
		std::unordered_map<std::string, tinygltf::Model*> _gltfMap{};
		tinygltf::TinyGLTF _loader;
	public:
		App *app;
	public:
		ModelMgr() = default;
		tinygltf::Model* getGltf(const std::string name);
		qb::Model* getModel(const std::string name);

		void init(App *app);
		void destroy();
	};

	class Model {
	public:
		std::string name;
		App* app;
		tinygltf::Model* gltf;
	public: // gltf
		struct Node;

		struct Primitive {
			uint32_t firstIndex;
			uint32_t indexCount;
			Primitive(uint32_t firstIndex, uint32_t indexCount) :firstIndex(firstIndex), indexCount(indexCount) {};
		};

		struct Mesh {
			std::string name;
			std::vector<Primitive*> primitives;

			struct Uniform {
				glm::mat4 mat;
				glm::mat4 joinMat[max_bones_per_mesh]{};
			} uniform;

			qb::Buffer* uniBuf;
			qb::Descriptor* descriptor;

			Mesh(glm::mat4 mat) {
				uniform.mat = mat;
			}
		};

		struct Skin {
			std::string name;
			Node* skeletonRoot = nullptr;
			std::vector<glm::mat4> inverseBindMat{};
			std::vector<Node*> joints{};
		};

		struct Node {
			Node* parent;
			int32_t index;
			std::vector<Node*> children{};
			glm::mat4 mat;
			std::string name;
			Mesh *mesh;
			Skin *skin;
			int32_t skinIndex = -1;
			glm::vec3 translation{};
			glm::vec3 scale{ 1.0f };
			glm::quat rotation{};
			glm::mat4 localMat();
			glm::mat4 globalMat();
			void update();
		};

		struct AnimationChannel {
			enum PathType { TRANSLATION, ROTATION, SCALE };
			PathType path;
			Node* node;
			uint32_t samplerIndex;
		};

		struct AnimationSampler {
			enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
			InterpolationType interpolation;
			std::vector<float> inputs{};
			std::vector<glm::vec4> outputsVec4{};
		};

		struct Animation {
			std::string name;
			std::vector<AnimationSampler> samplers;
			std::vector<AnimationChannel> channels;
			float start = std::numeric_limits<float>::max();
			float end = std::numeric_limits<float>::min();
		};

		struct Vertex {
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 texCoord;
			glm::u16vec4 joint0;
			glm::vec4 weight0;
			vertex_desc(qb::Model::Vertex, 0, VK_VERTEX_INPUT_RATE_VERTEX,
				{ 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(qb::Model::Vertex, pos) },
				{ 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(qb::Model::Vertex, normal) },
				{ 2, VK_FORMAT_R32G32_SFLOAT, offsetof(qb::Model::Vertex, texCoord) },
				{ 3, VK_FORMAT_R16G16B16A16_UINT, offsetof(qb::Model::Vertex, joint0) },
				{ 4, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(qb::Model::Vertex, weight0) },
			);
		};
		Buffer* vertexBuf;
		Buffer* indexBuf;
		std::vector<uint32_t> indices{};
		std::vector <qb::Model::Vertex> vertices{};
		//std::vector<Node*> nodes{};
		std::vector<Node*> linearNodes{};
		std::vector<Skin*> skins{};
		std::vector<qb::Image*> images{};
		//todo material
		std::vector<Animation> animations{};
		std::vector<std::string> extensions{};

		Node* rootNode;

		Animation* currAnimaiton;
		std::unordered_map<std::string, Animation*> animMap;
		std::vector<Mesh*> linearMeshes{};
	private:
		void _loadNode(qb::Model::Node* parent, const tinygltf::Node& node, uint32_t nodeIndex);
		void _loadSkins();
		void _loadImages();
		void _loadAnimations();
		Node* _findNode(qb::Model::Node* parent, uint32_t index);
		Node* _nodeFromIndex(uint32_t index);
	public:
		void setAnimation(std::string name);
		void updateAnimation(float time);
		void init(App* app, std::string name);
		void destroy();
		void build();
	};
};