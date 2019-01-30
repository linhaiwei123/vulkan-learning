#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(binding = 0) uniform UniformBuffer {
    mat4 model;
    mat4 view;
    mat4 proj;
} uniBuf;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = uniBuf.proj * uniBuf.view * uniBuf.model * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}