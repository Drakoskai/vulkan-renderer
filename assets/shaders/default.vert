#version 450 core
#pragma shader_stage(vertex)

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 mvp;
	mat4 world;
} ubo;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec4 outNormal;

void main() {
    gl_Position = ubo.mvp * vec4(inPosition.xyz, 1.0);
	outNormal = ubo.world * vec4(normalize(inNormal).xyz, 1.0);
	outTexCoord = inTexCoord;
}
