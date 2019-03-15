#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uvec4 inJoint0;
layout(location = 4) in vec4 inWeight0;

#define max_bone_per_mesh 64

layout(binding = 0) uniform modelUniformBuffer {
	mat4 mat;
	mat4 joinMat[max_bone_per_mesh];
	uint jointCount;
} modelUniBuf;

layout(location = 0) out vec3 fragColor;

void main() {
	mat4 boneTransform = mat4(1.0f);
	if(modelUniBuf.jointCount > 0) 
	{
		boneTransform = modelUniBuf.joinMat[inJoint0[0]] * inWeight0[0];
		boneTransform += modelUniBuf.joinMat[inJoint0[1]] * inWeight0[1];
		boneTransform += modelUniBuf.joinMat[inJoint0[2]] * inWeight0[2];
		boneTransform += modelUniBuf.joinMat[inJoint0[3]] * inWeight0[3];
	}

	gl_Position = modelUniBuf.mat * boneTransform * vec4(inPosition,1.0f);
	fragColor = vec3(1.0f, 1.0f, 1.0f);
}