#version 450

layout(binding = 0) uniform UniformBufferObject {
	mat4 inverse_view;
	mat4 view_proj;
	vec3 light_direction;
	vec3 light_color;
	vec3 ambient_color;
} ub;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(set = 1, binding = 1) uniform MaterialUniformBufferObject 
{ 
	float roughness; 
	vec3 diffuse_color;
	vec3 specular_color;
} mat_ub;


layout(location = 0) in vec3 fragWorldPosition;
layout(location = 1) in vec3 fragWorldNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	float light_intensity = max(dot(normalize(fragWorldNormal), normalize(ub.light_direction)), 0);
	vec3 deffuse_light = light_intensity * ub.light_color;

	vec3 camera_position = ub.inverse_view[3].xyz;
	vec3 view_direction = normalize(camera_position - fragWorldPosition);
	vec3 half_angle = normalize(ub.light_direction + view_direction);
	vec3 surface_normal = normalize(fragWorldNormal);
	float blinn_term = dot(surface_normal, half_angle);
	blinn_term = clamp(blinn_term, 0, 1);
	blinn_term = pow(blinn_term, mat_ub.roughness);

	//vec3 specular_light = ub.light_color.xyz * blinn_term;
	vec3 specular_light = specular_color * ub.light_color.xyz * blinn_term;

	vec3 base_color = mat_ub.diffuse_color * texture(texSampler, fragTexCoord).rgb;

	outColor = vec4(ub.ambient_color * base_color + base_color * deffuse_light + specular_light, 1.0);
}