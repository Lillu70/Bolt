#version 450

layout(binding = 0) uniform UniformBufferObject {
	mat4 inverse_view;
	mat4 view_proj;
	vec3 light_direction;
	vec3 light_color;
	vec3 ambient_color;
} ub;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(set = 1, binding = 1) uniform MaterialUniformBufferObject { float roughness; } mat_ub;


layout(location = 0) in vec3 fragWorldPosition;
layout(location = 1) in vec3 fragWorldNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 base_color =  texture(texSampler, fragTexCoord).rgb;
	outColor = vec4(base_color, 1.0);
}