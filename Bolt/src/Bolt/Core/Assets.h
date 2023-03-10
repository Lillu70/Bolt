#pragma once

#include "Assets_Resource_Injector.h"
#include "Bolt_Types.h"
#include "Obj_Loader.h"
#include "Assert.h"
#include "Maths.h"

#include <unordered_map>
#include <string>

namespace Bolt
{

	class Assets final : public Assets_Resource_Injector
	{
	public:
		Assets(Resource_Factory* renderer_resource_factory) : m_renderer_resource_factory(renderer_resource_factory) {}
		~Assets();
		
		std::vector<std::pair<std::string, std::string>>& Load_Model_File(const std::string& file_path, Bolt::Shader shader = {}, bool offset_by_origin = false);

		Raw_Shader* Shader(const std::string& name);

		Mesh* Mesh(const std::string& mesh_name);

		Texture* Texture(const std::string& texture_path);

		Material* Material(const std::string& material_name);

		glm::vec3 Mesh_Origin(const std::string& name);

		//Create calls.
	public:
		Scene_Descriptor* Create_Scene_Descriptor(u32 main_pass_index, u32 subpass_index);

		Render_Pass* Create_Render_Pass(u32 main_pass_index, u32 subpass_index, Clear_Method color_clear, Clear_Method depth_clear);
		
		Bolt::Material* Create_Material(const std::string& material_name, Material_Properties properties, Bolt::Texture* texture, Bolt::Shader shader = {});
		
		void Create_Shader(const std::string& name, const std::string& frag_path, const std::string& vert_path);

	private:
		void Push_Mesh(const std::string& name, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) override;
		
		void Push_Material(const std::string& model_name, const std::string& material_name, Material_Data& material_data) override;

	private:
		Resource_Factory* m_renderer_resource_factory;
		Obj_Loader m_obj_loader;
		Bolt::Shader m_active_shader;
		bool m_offset_mesh_by_origin = false;

		std::string m_mesh_path_prefix = "_Models/";
		std::string m_texture_path_prefix = "_Textures/";
		std::string m_shader_path_prefix = "_Shaders/";

		std::unordered_map<std::string, Bolt::Mesh>			m_mesh_map;
		std::unordered_map<std::string, glm::vec3>			m_mesh_origin_map;
		std::unordered_map<std::string, Bolt::Material>		m_material_map;
		std::unordered_map<std::string, Bolt::Texture>		m_texture_map;
		std::unordered_map<std::string, Bolt::Raw_Shader>	m_shader_map;
		std::unordered_map<u32, Bolt::Render_Pass>			m_render_pass_map;
		std::unordered_map<u32, Bolt::Scene_Descriptor>		m_scene_descriptor_map;
		
		std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> m_model_map;
	};
}



