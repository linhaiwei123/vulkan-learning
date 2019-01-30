#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout (input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput inputColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform PushConst {
	vec2 speed;
} pushConst;

layout(location = 0) out vec4 outColor;


void main() {
	vec4 mask = subpassLoad(inputColor).rgba;
    vec4 texColor = texture(texSampler, fragTexCoord + pushConst.speed);
	outColor = mix(texColor, texColor + mask, mask.a);
}