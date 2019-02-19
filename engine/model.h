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
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <array>
namespace qb {
	class App;
	class Buffer;
	class Model;
	class ModelMgr {
	private:
		Assimp::Importer _importer;
		std::unordered_map<std::string, const aiScene*> _aiSceneMap{};
		std::unordered_map<std::string, Model*> _modelMap{};
	public:
		App *app;
	public:
		ModelMgr() = default;
		
		const aiScene* getAssimpScene(const std::string name);
		Model* getModel(const std::string name);

		void init(App *app);
		void destroy();
	};

	struct ModelVertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::vec3 normal;
		float boneWeights[max_bones_per_vertex];
		uint32_t boneIDs[max_bones_per_vertex];
		vertex_desc(ModelVertex, model_vertex_binding_id, VK_VERTEX_INPUT_RATE_VERTEX,
			{ 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ModelVertex, pos) },
			{ 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ModelVertex, color) },
			{ 2, VK_FORMAT_R32G32_SFLOAT, offsetof(ModelVertex, texCoord) },
			{ 3, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ModelVertex, normal) },
			{ 4, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(ModelVertex, boneWeights) },
			{ 5, VK_FORMAT_R32G32B32A32_UINT, offsetof(ModelVertex, boneIDs) },
		);
	};

	struct VertexBoneData {
		std::array<uint32_t, max_bones_per_vertex> ids;
		std::array<float, max_bones_per_vertex> weights;

		void add(uint32_t boneId, float weight) {
			for (uint32_t i = 0; i < max_bones_per_vertex; i++) {
				if (weights[i] == 0.0f) {
					ids[i] = boneId;
					weights[i] = weight;
					return;
				}
			}
		}
	};

	struct BoneInfo {
		aiMatrix4x4 offset{};
		aiMatrix4x4 finalTransformation{};
	};

	class Model {
	public:
		std::string name;
		App* app;
		std::vector<ModelVertex> modelVertices;
		Buffer* modelVertexBuf;
		std::vector<uint16_t> modelIndices;
		Buffer* modelIndexBuf;
		const aiScene* scene;
		// skin mesh 
		std::unordered_map<std::string, uint32_t> boneIdMaps;
		std::unordered_map<std::string, uint32_t> animIdMaps;
		std::vector<BoneInfo> boneInfos;
		uint32_t numBones = 0;
		aiMatrix4x4 globalInverseTransform;
		std::vector<VertexBoneData> vertexBoneDatas;
		std::vector<aiMatrix4x4> boneTransforms;
		float animationSpeed = 10.0f;
		aiAnimation* animation;
	public:
		void setAnimation(const std::string name) {
			auto it = animIdMaps.find(name);
			if (it == animIdMaps.end())
				assert(0);
			uint32_t id = it->second;
			animation = scene->mAnimations[id];
		}
		void loadBones(const aiMesh* mesh, uint32_t vertexOffset, std::vector<VertexBoneData>& vertexBoneDatas) {
			for (uint32_t i = 0; i < mesh->mNumBones; i++) {
				uint32_t boneId = 0;
				assert(mesh->mNumBones <= max_bones_per_mesh);

				std::string name(mesh->mBones[i]->mName.data);
				if (boneIdMaps.find(name) == boneIdMaps.end()) {
					boneId = numBones;
					numBones++;
					BoneInfo boneInfo;
					boneInfos.push_back(boneInfo);
				}
				else {
					boneId = boneIdMaps[name];
				}

				boneIdMaps[name] = boneId;
				boneInfos[boneId].offset = mesh->mBones[i]->mOffsetMatrix;

				for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
					uint32_t vertexId = vertexOffset + mesh->mBones[i]->mWeights[j].mVertexId;
					float weight = mesh->mBones[i]->mWeights[j].mWeight;
					vertexBoneDatas[vertexId].add(boneId, weight);
				}
			}
			boneTransforms.resize(numBones);
		}
		void update(float time) {
			time *= animationSpeed;
			float ticksPerSecond = (float)(animation->mTicksPerSecond != 0? animation->mTicksPerSecond : 60.0f);
			float timeInTicks = time * ticksPerSecond;
			float animationTime = fmod(timeInTicks, (float)animation->mDuration);
			aiMatrix4x4 identity = aiMatrix4x4();
			readNodeHierarchy(animationTime, scene->mRootNode, identity);
			for (uint32_t i = 0; i < boneTransforms.size(); i++) {
				boneTransforms[i] = boneInfos[i].finalTransformation;
			}
		}
	private:
		const aiNodeAnim* findNodeAnim(const aiAnimation* animation, const std::string nodeName) {
			for (uint32_t i = 0; i < animation->mNumChannels; i++) {
				const aiNodeAnim* nodeAnim = animation->mChannels[i];
				if (std::string(nodeAnim->mNodeName.data) == nodeName) {
					return nodeAnim;
				}
			}
			return nullptr;
		}
		aiMatrix4x4 interpolateTranslation(float time, const aiNodeAnim* pNodeAnim) {
			aiVector3D translation;
			if (pNodeAnim->mNumPositionKeys == 1) {
				translation = pNodeAnim->mPositionKeys[0].mValue;
			}
			else {
				uint32_t frameIndex = 0;
				for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
					if (time < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
						frameIndex = i;
						break;
					}
				}

				aiVectorKey currentFrame = pNodeAnim->mPositionKeys[frameIndex];
				aiVectorKey nextFrame = pNodeAnim->mPositionKeys[(frameIndex + 1) % pNodeAnim->mNumPositionKeys];

				float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);
				const aiVector3D& start = currentFrame.mValue;
				const aiVector3D& end = nextFrame.mValue;

				translation = (start + delta * (end - start));
			}
			aiMatrix4x4 mat;
			aiMatrix4x4::Translation(translation, mat);
			/*glm::mat4 temp = glm::translate(glm::mat4(1.0f), glm::make_vec3(&translation.x));
			aiMatrix4x4 mat = GLMMat4ToAi(temp).Transpose();*/
			return mat;
		}
		aiMatrix4x4 interpolateRotation(float time, const aiNodeAnim* pNodeAnim) {
			aiQuaternion rotation;
			if (pNodeAnim->mNumRotationKeys == 1) {
				rotation = pNodeAnim->mRotationKeys[0].mValue;
			}
			else {
				uint32_t frameIndex = 0;
				for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
					if (time < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
						frameIndex = i;
						break;
					}
				}
				aiQuatKey currentFrame = pNodeAnim->mRotationKeys[frameIndex];
				aiQuatKey nextFrame = pNodeAnim->mRotationKeys[(frameIndex + 1) % pNodeAnim->mNumRotationKeys];

				float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);
				const aiQuaternion& start = currentFrame.mValue;
				const aiQuaternion& end = nextFrame.mValue;
				//aiQuaternion::Interpolate(rotation, start, end, delta);
				glm::quat temp = glm::slerp(glm::quat(start.w,start.x,start.y,start.z), glm::quat(end.w, end.x, end.y, end.z), delta);
				rotation = aiQuaternion(temp.w, temp.x, temp.y, temp.z);
				//rotation.Normalize();
			}

			aiMatrix4x4 mat(rotation.GetMatrix());
			return mat;
		}
		aiMatrix4x4 interpolateScale(float time, const aiNodeAnim* pNodeAnim) {
			aiVector3D scale;

			if (pNodeAnim->mNumScalingKeys == 1) {
				scale = pNodeAnim->mScalingKeys[0].mValue;
			}
			else {
				uint32_t frameIndex = 0;
				for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
					if (time < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
						frameIndex = i;
						break;
					}
				}

				aiVectorKey currentFrame = pNodeAnim->mScalingKeys[frameIndex];
				aiVectorKey nextFrame = pNodeAnim->mScalingKeys[(frameIndex + 1) % pNodeAnim->mNumScalingKeys];

				float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

				const aiVector3D& start = currentFrame.mValue;
				const aiVector3D& end = nextFrame.mValue;

				scale = (start + delta * (end - start));
			}

			aiMatrix4x4 mat;
			aiMatrix4x4::Scaling(scale, mat);
			return mat;
		}
		void readNodeHierarchy(float animationTime, const aiNode* pNode, const aiMatrix4x4& parentTransform) {
			std::string nodeName(pNode->mName.data);
			aiMatrix4x4 nodeTransformation(pNode->mTransformation);
			const aiNodeAnim* pNodeAnim = findNodeAnim(animation, nodeName);

			if (pNodeAnim) {
				aiMatrix4x4 matScale = interpolateScale(animationTime, pNodeAnim);
				aiMatrix4x4 matRotation = interpolateRotation(animationTime, pNodeAnim);
				aiMatrix4x4 matTranslate = interpolateTranslation(animationTime, pNodeAnim);

				nodeTransformation = matTranslate * matRotation * matScale;
				
			}

			aiMatrix4x4 globalTransformation = parentTransform * nodeTransformation;

			if (boneIdMaps.find(nodeName) != boneIdMaps.end()) {
				uint32_t boneId = boneIdMaps[nodeName];
				boneInfos[boneId].finalTransformation = globalInverseTransform * globalTransformation * boneInfos[boneId].offset;
			}

			for (uint32_t i = 0; i < pNode->mNumChildren; i++) {
				readNodeHierarchy(animationTime, pNode->mChildren[i], globalTransformation);
			}
		}

		static aiMatrix4x4 GLMMat4ToAi(glm::mat4 mat)
		{
			return aiMatrix4x4(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
				mat[1][0], mat[1][1], mat[1][2], mat[1][3],
				mat[2][0], mat[2][1], mat[2][2], mat[2][3],
				mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
		}
	public:
		void init(App* app, std::string name);
		void destroy();
		void build();
	};

};