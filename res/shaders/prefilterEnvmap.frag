#version 450

layout(location = 0) in vec3 inPos;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform samplerCube samplerEnv;

layout(push_constant) uniform PushConsts {
	layout(offset = 64) float roughness;
	layout(offset = 68) uint numSamples;
} consts;

const float PI = 3.1415926536;

float random(vec2 co) {
	float a = 12.9898;
	float b = 78.233;
	float c = 43758.5453;
	float dt = dot(co.xy, vec2(a, b));
	float sn = mod(dt, 3.14);
	return fract(sin(sn) * c);
}

vec2 hammersley2d(uint i, uint n) {
	uint bits = (i << 16u) | (i >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	float rdi = float(bits) * 2.3283064365386963e-10;
	return vec2(float(i) /float(n), rdi);
}

vec3 importanceSample_GGX(vec2 Xi, float roughness, vec3 normal) {

	// map to 2d point to hemisphere with spread based on roughenss
	float alpha = roughness * roughness;
	float phi = 2.0 * PI * Xi.x + random(normal.xz) * 0.1;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	// tangent space
	vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangentX = normalize(cross(up, normal));
	vec3 tangentY = normalize(cross(normal, tangentX));

	// convert to world space
	return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
}

float D_GGX(float NdotH, float roughness) {
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom * denom);
}

vec3 prefilterEnvMap(vec3 r, float roughness) {
	vec3 n = r;
	vec3 v = r;
	vec3 color = vec3(0.0);
	float totalWeight = 0.0;
	float envMapDim = float(textureSize(samplerEnv, 0));
	for(uint i = 0u; i < consts.numSamples; i++) {
		vec2 Xi = hammersley2d(i, consts.numSamples);
		vec3 h = importanceSample_GGX(Xi, roughness, n);
		vec3 l = 2.0 * dot(v, h) * h - v;
		float NdotL = clamp(dot(n, l), 0.0, 1.0);
		if(NdotL > 0.0) {
			float NdotH = clamp(dot(n, h), 0.0, 1.0);
			float VdotH = clamp(dot(v, h), 0.0, 1.0);

			float pdf = D_GGX(NdotH, roughness) * NdotH / (4.0 * VdotH) + 0.0001;
			float omegaS = 1.0 / (float(consts.numSamples) * pdf);
			float omegaP = 4.0 * PI / (6.0 * envMapDim * envMapDim);
			float mipLevel = roughness == 0.0 ? 0.0 : max(0.5 * log2(omegaS / omegaP) + 1.0, 0.0);
			color += textureLod(samplerEnv, l, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	return (color / totalWeight);
}

void main() {
	vec3 n = normalize(inPos);
	outColor = vec4(prefilterEnvMap(n, consts.roughness), 1.0);
}