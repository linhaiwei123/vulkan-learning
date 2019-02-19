#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec2 inNormal;
layout(location = 4) in vec4 inBoneWeights;
layout(location = 5) in uvec4 inBoneIds;

#define max_bone_per_mesh 64

layout(binding = 0) uniform modelUniformBuffer {
	mat4 model;
    mat4 view;
    mat4 proj;
	mat4 bones[max_bone_per_mesh];
} modelUniBuf;

layout(location = 0) out vec3 fragColor;

void main() {
	mat4 boneTransform = modelUniBuf.bones[inBoneIds[0]] * inBoneWeights[0];
		boneTransform += modelUniBuf.bones[inBoneIds[1]] * inBoneWeights[1];
		boneTransform += modelUniBuf.bones[inBoneIds[2]] * inBoneWeights[2];
		boneTransform += modelUniBuf.bones[inBoneIds[3]] * inBoneWeights[3];

	gl_Position = modelUniBuf.proj * modelUniBuf.view * modelUniBuf.model * boneTransform * vec4(inPosition,1.0f);
	fragColor = inColor;
}