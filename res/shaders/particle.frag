#version 450

layout(binding = 0) uniform sampler2D samplerColorMap;
layout(binding = 1) uniform sampler1D samplerGradientRamp;

layout(location = 1) in float inGradient;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 gradient = texture(samplerGradientRamp, inGradient).rrr;
	vec4 color = texture(samplerColorMap, gl_PointCoord);
	outColor = vec4(color.rgb, gradient);
}