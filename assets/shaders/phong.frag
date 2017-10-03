#version 450 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform sampler2D sampler;

layout(binding = 2) uniform Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 emissive;
	vec3 specular;
	float reflection;
	float opacity;
	float shininess;
} material;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inViewVec;
layout(location = 3) in vec3 inLightVec;

layout(location = 0) out vec4 outColor;

vec3 applyNormalMap(vec3 normal, vec3 normSample){
    normSample = normSample * 2.0 - 1.0;
    vec3 up = normalize(vec3(0.001, 1, 0.001));
    vec3 surftan = normalize(cross(normal, up));
    vec3 surfbinor = cross(normal, surftan);
    return normalize(normSample.y * surftan + normSample.x * surfbinor + normSample.z * normal);
}

void main() {			
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
		
	vec3 ambient = material.ambient;
	vec4 color;
	color = texture(sampler, inTexCoord);	
	if ((color.r >= 0.9) || (color.g >= 0.9) || (color.b >= 0.9)) {
		ambient *= color.rgb * 0.25;
	} else {
		ambient *= color.rgb;
	}
	
	vec3 diffuse = material.diffuse * max(dot(N, L), 0.0) * color.rgb;
	//blinn
	vec3 halfwayDir = normalize(L + V);  
	vec3 specular = pow(max(dot(N, halfwayDir), 0.0), 32.0) * material.specular;
	//phong
	//vec3 R = reflect(-L, N);
	//vec3 specular = pow(max(dot(R, V), 0.0), 8.0) * material.specular;
		
	outColor = vec4(ambient + diffuse + specular, material.opacity);	
}
