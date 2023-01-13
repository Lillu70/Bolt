#version 450

layout(binding = 0) uniform UniformBufferObject {
	mat4 inverse_view;
	mat4 view_proj;
	vec3 light_direction;
	vec3 light_color;
	vec3 ambient_color;
} ub;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragWorldPosition;
layout(location = 1) out vec3 fragWorldNormal;
layout(location = 2) out vec2 fragTexCoord;

layout( push_constant ) uniform constants
{
	mat4 model_matrix;
	mat4 normal_matrix;
} pc;


void main()
{
	vec4 world_position = pc.model_matrix * vec4(inPosition, 1.0);
	fragWorldPosition = world_position.xyz;

	gl_Position = ub.view_proj * pc.model_matrix * vec4(inPosition, 1.0);

	fragWorldNormal = normalize(mat3(pc.normal_matrix) * inNormal);

	fragTexCoord = inTexCoord;
}