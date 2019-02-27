#version 450
layout(location = 0) in vec2 fragTexCoord;

layout(binding = 3) uniform usampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0f, 0.0f, 0.0f, float(texture(texSampler, fragTexCoord).r)/255.0f);
}