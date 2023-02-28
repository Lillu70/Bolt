#version 450

layout(binding = 0) uniform UniformBufferObject {
	mat4 inverse_view;
	mat4 proj_view;
	vec3 light_direction;
	vec3 light_color;
	vec3 ambient_color;
} ub;

layout(location = 0) in vec3 in_offset;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_coord;


layout(location = 0) out vec2 out_frag_tex_coord;
layout(location = 1) out vec3 out_frag_color;

layout( push_constant ) uniform constants
{
	mat4 model_center;
	mat4 normal_matrix;
} pc;


void main()
{
	mat4 view_matrix = inverse(ub.inverse_view);

	vec3 camera_right_worldspace = {view_matrix[0][0], view_matrix[1][0], view_matrix[2][0]};
	vec3 camera_up_worldspace = {view_matrix[0][1], view_matrix[1][1], view_matrix[2][1]};
	vec3 camera_depth_worldspace = cross(camera_right_worldspace, camera_up_worldspace);

	vec3 world_position = pc.model_center[0].xyz + camera_right_worldspace * in_offset.x * pc.model_center[1][0] + camera_up_worldspace * in_offset.y * pc.model_center[1][1];
	
	out_frag_tex_coord = in_tex_coord;
	out_frag_color = pc.model_center[3].xyz;
	gl_Position = ub.proj_view * vec4(world_position, 1.0);
}