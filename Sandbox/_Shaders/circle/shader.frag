#version 450

layout(binding = 0) uniform UniformBufferObject {
	mat4 inverse_view;
	mat4 proj_view;
	vec3 light_direction;
	vec3 light_color;
	vec3 ambient_color;
} ub;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(set = 1, binding = 1) uniform MaterialUniformBufferObject 
{ 
	float roughness; 
	float transparensy;
	vec3 diffuse_color;
	vec3 specular_color;
} mat_ub;


layout(location = 0) in vec2 frag_tex_coord;


layout(location = 0) out vec4 outColor;

void main()
{
	vec2 uv = (frag_tex_coord - 0.5) * 2;
	

	if( dot(uv,uv) < 1)
	{
		outColor.rgba = vec4(0.3,0,0.3,0.8);
	}
	else
	{
		outColor.rgba = vec4(0,0,0,0);
	}

}