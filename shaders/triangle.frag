#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D tex2dSampler;
layout (input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput inputColor;
layout(binding = 3) uniform sampler2D compSampler;
layout(binding = 4) uniform sampler2DArray tex2dArrSampler;


layout(push_constant) uniform PushConst {
	vec2 speed;
} pushConst;

layout(location = 0) out vec4 outColor;


void main() {
	vec4 mask = subpassLoad(inputColor).rgba;
    vec4 texColor = texture(tex2dSampler, fragTexCoord + pushConst.speed);
	vec4 compColor = texture(compSampler, fragTexCoord);
	int arrayIndex = int(pushConst.speed) % 3;
	vec4 texArrColor = texture(tex2dArrSampler, vec3(fragTexCoord, arrayIndex));
	outColor = mix(texColor, texColor + mask, mask.a) * compColor * texArrColor;
	
}