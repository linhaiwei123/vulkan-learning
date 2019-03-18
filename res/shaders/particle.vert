#version 450

layout(location = 0) in vec2 inPos;
layout(location = 2) in vec4 inGradient;

layout(location = 1) out float outGradient;

void main() {
	gl_PointSize = 8.0;
	outGradient = inGradient.x;
	gl_Position = vec4(inPos.xy, 1.0, 1.0);
}