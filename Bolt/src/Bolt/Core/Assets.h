#pragma once

#include "../Vulkan/Renderer_Resource_Factory.h"

#include <unordered_map>
#include <string>

#include "Assert.h"

namespace Bolt
{

	class Assets final
	{
	
	public:
		Assets(Resource_Factory* renderer_resource_factory) : m_renderer_resource_factory(renderer_resource_factory) 
		{
		
		}

		~Assets()
		{
			for (auto& [file_path, mesh] : m_mesh_map)
				m_renderer_resource_factory->Destroy(mesh);

			for (auto& [file_path, texture] : m_texture_map)
				m_renderer_resource_factory->Destroy(texture);

			for (auto& [file_path, shader] : m_shader_map)
				m_renderer_resource_factory->Destroy(shader);

			for (auto& [file_path, material] : m_material_map)
				m_renderer_resource_factory->Destroy(material);
		}
		
		Mesh* Mesh(const char* file_path)
		{
			auto find = m_mesh_map.find(file_path);
			if (find != m_mesh_map.end())
				return &find->second;

			Bolt::Mesh& mesh = m_mesh_map[file_path];

			std::string full_path = m_mesh_path_prefix + file_path;
			m_renderer_resource_factory->Load_Mesh(full_path.c_str(), mesh);
			
			return &mesh;
		}

		Texture* Texture(const char* file_path)
		{
			auto find = m_texture_map.find(file_path);
			if (find != m_texture_map.end())
				return &find->second;

			Bolt::Texture& texture = m_texture_map[file_path];

			std::string full_path = m_texture_path_prefix + file_path;
			m_renderer_resource_factory->Load_Texture(full_path.c_str(), texture);

			return &texture;
		}

		Shader* Shader(const char* frag_path, const char* vert_path)
		{
			std::string map_key = std::string(frag_path) + vert_path;

			auto find = m_shader_map.find(map_key);
			if (find != m_shader_map.end())
				return &find->second;

			Bolt::Shader& shader = m_shader_map[map_key];
			std::string full_path_frag = m_shader_path_prefix + frag_path;
			std::string full_path_vert = m_shader_path_prefix + vert_path;
			m_renderer_resource_factory->Load_Shader(full_path_frag.c_str(), full_path_vert.c_str(), shader);

			return &shader;
		}

		Material* Material(const char* name)
		{
			auto find = m_material_map.find(name);
			if (find != m_material_map.end())
				return &find->second;

			ERROR(std::string("Material with name [") + name + "] doesn't exists!");
		}

		void Create_Material(const char* name, Material_Properties properties, Bolt::Texture* texture, Bolt::Shader* shader = nullptr)
		{
			ASSERT(texture, "Texture is a nullptr");

			auto find = m_material_map.find(name);
			if (find != m_material_map.end())
				ERROR(std::string("Material with name [") + name + "] already exists!");

			Bolt::Material& material = m_material_map[name];
			m_renderer_resource_factory->Create_Material(properties, *texture, material, shader ? std::optional<Bolt::Shader>( *shader ) : std::nullopt);
		}

	private:
		

	private:
		Resource_Factory* m_renderer_resource_factory;

		std::string m_mesh_path_prefix = "_Models/";
		std::string m_texture_path_prefix = "_Textures/";
		std::string m_shader_path_prefix = "_Shaders/";

		std::unordered_map<const char*, Bolt::Mesh>		m_mesh_map;
		std::unordered_map<const char*, Bolt::Texture>	m_texture_map;
		std::unordered_map<const char*, Bolt::Material> m_material_map;
		std::unordered_map<std::string, Bolt::Shader>	m_shader_map;
	};
}



