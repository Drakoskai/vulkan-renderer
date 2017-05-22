#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
	mat4 view;
	mat4 proj;
	mat4 world;
    vec3 lightpos;
} ubo;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outViewVec;
layout (location = 3) out vec3 outLightVec;

void main() {
	outTexCoord = inTexCoord;
	//outNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
	outNormal = mat3(ubo.model) * inNormal;
	mat4 modelView = ubo.view * ubo.model;
	mat4 worldView = ubo.view * ubo.world;
	vec4 pos = modelView * vec4(inPosition.xyz, 1.0);
	gl_Position = ubo.proj * pos;
	
	vec4 lightPos = worldView * vec4(ubo.lightpos, 1.0);
	outLightVec = normalize(lightPos.xyz - pos.xyz);
	outViewVec = -pos.xyz;
	
}