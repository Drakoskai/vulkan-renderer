#version 450 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inViewVec;
layout (location = 3) in vec3 inLightVec;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 ambient = vec3(0.5880, 0.5880, 0.5880);
	vec4 color = texture(texSampler, inTexCoord);
	if ((color.r >= 0.9) || (color.g >= 0.9) || (color.b >= 0.9)) {
		ambient *= color.rgb * 0.25;
	} else {
		ambient *= color.rgb;
	}
	
	
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);
	
	vec3 diffuse = vec3(0.5880, 0.5880, 0.5880) * max(dot(N, L), 0.0) * color.rgb;
	vec3 specular = pow(max(dot(R, V), 0.0), 8.0) * vec3(0.75);
	
	outColor = vec4(ambient + diffuse + specular, 1.0);	
}