#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in uvec4 inJoint0;
layout(location = 4) in vec4 inWeight0;

layout(set = 0, binding = 0) uniform MvpUbo {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 camPos;
} mvpUbo;

#define MAX_NUM_JOINTS 128

layout(set = 1, binding = 0) uniform NodeUbo {\
	mat4 mat;
	mat4 jointMatrix[MAX_NUM_JOINTS];
	float jointCount;
} nodeUbo;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;

void main()
{
	vec4 locPos;
	if(nodeUbo.jointCount > 0.0) {
		mat4 skinMat = inWeight0.x * nodeUbo.jointMatrix[int(inJoint0.x)]
						+ inWeight0.y * nodeUbo.jointMatrix[int(inJoint0.y)]
						+ inWeight0.z * nodeUbo.jointMatrix[int(inJoint0.z)]
						+ inWeight0.w * nodeUbo.jointMatrix[int(inJoint0.w)];
		mat4 locMat = mvpUbo.model * nodeUbo.mat * skinMat;
		locPos = locMat * vec4(inPos, 1.0f);
		outNormal = normalize(transpose(inverse(mat3(locMat))) * inNormal);
	} else{
		mat4 locMat = mvpUbo.model * nodeUbo.mat;
		locPos = locMat * vec4(inPos, 1.0);
		outNormal = normalize(transpose(inverse(mat3(locMat))) * inNormal);
	}
	outWorldPos = locPos.xyz / locPos.w;
	outUV = inUV;
	gl_Position = mvpUbo.proj * mvpUbo.view * vec4(outWorldPos, 1.0f);
}


