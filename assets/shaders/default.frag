#version 450 core
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(texSampler, inTexCoord);
}
