#version 450
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(binding = 0) uniform mvpUniformBuffer {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvpUniBuf;

layout(binding = 1) uniform chUniformBuffer {
    vec2 size;
    vec2 bearing;
} chUniBuf;

layout(binding = 2) uniform advanceUniformBuffer {
    vec2 advance;
} advUniBuf;

layout(location = 0) out vec2 fragTexCoord;

void main() {
	int index = gl_VertexIndex;
	// character
	float xpos = inPosition.x + chUniBuf.bearing.x;
	// advance
	xpos += advUniBuf.advance.x;
	float ypos = inPosition.y - (chUniBuf.size.y - chUniBuf.bearing.y);
	float w = chUniBuf.size.x;
	float h = chUniBuf.size.y;
	switch(index){
		case 0:{
			ypos += h;
			break;
		}
		case 1:{
			break;
		}
		case 2:{
			xpos += w;
			break;
		}
		case 3:{
			ypos += h;
			xpos += w;
			break;
		}
	}
	
	// mvp
    gl_Position = mvpUniBuf.proj * mvpUniBuf.view * mvpUniBuf.model * vec4(xpos,ypos, 0.0, 1.0);
	fragTexCoord = inTexCoord;
}