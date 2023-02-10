#pragma once
#include "Vk_Resources.h"


namespace Bolt
{
	class Resource_Factory
	{
	public:
		virtual void Load_Texture(const char* file_path, Texture& output_texture) = 0;
		virtual void Destroy(Texture& texture) = 0;

		virtual void Load_Shader(const char* frag_file_path, const char* vert_file_path, Raw_Shader& output_shader) = 0;
		virtual void Destroy(Raw_Shader& shader) = 0;

		virtual void Create_Material(const Material_Properties& properties, const Texture& texture, Material& output_material, const Shader shader) = 0;
		virtual void Destroy(Material& material) = 0;

		virtual void Load_Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Mesh& output_mesh) = 0;
		virtual void Destroy(Mesh& mesh) = 0;

		virtual void Create_Render_Pass(Render_Pass& output, Clear_Method color_clear_method, Clear_Method depth_clear_method) = 0;
		virtual void Destroy(Render_Pass& render_pass) = 0;

		virtual void Create_Scene_Descriptor(Scene_Descriptor& output) = 0;
		virtual void Destroy(Scene_Descriptor& scene_descriptor) = 0;
	};
}