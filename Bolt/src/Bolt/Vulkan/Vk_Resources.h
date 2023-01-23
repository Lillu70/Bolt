#pragma once

#include "Vk_Internal_Components.h"



namespace Bolt
{
	class Vk_Renderer;


	struct Shader
	{
		friend Vk_Renderer;

	private:
		static std::vector<char> Read_Byte_Code(const std::string& file_name);

	private:
		Shader_Modules m_shader_modules;
	};

	struct Image_Data
	{
		int32_t width = 0;
		int32_t height = 0;
		int32_t channels = 0;
		unsigned char* pixels = nullptr;

		void Free();
	};

	struct Texture
	{
		friend Vk_Renderer;

	private:
		static Image_Data Load_Image(const char* file_path);

	private:
		Image_Description m_image;
	};

	struct Material
	{
		friend Vk_Renderer;

	private:
		VkDescriptorSet m_descriptor_set = VK_NULL_HANDLE;
		VkPipeline m_pipeline = VK_NULL_HANDLE;

		Buffer_Description m_material_buffer;
	};

	struct Mesh
	{
		friend Vk_Renderer;

	private:
		Buffer_Description m_vertex_buffer;
		Buffer_Description m_index_buffer;
		uint32_t m_index_count = 0;
	};

	struct Render_Object
	{
		Render_Object() = default;
		Render_Object(Mesh* mesh, Material* material, glm::mat4 transform) : mesh(mesh), material(material), transform(transform) {}

		Mesh* mesh = nullptr;
		Material* material = nullptr;
		glm::mat4 transform = glm::mat4(0);
	};

	struct Enviroment_Data
	{
		glm::vec3 global_light_direction = glm::vec3(1, 3, 1);
		glm::vec3 global_light_color = glm::vec3(1);
		glm::vec3 amplient_color = glm::vec3(0.005, 0.005, 0.005);
	};
}

