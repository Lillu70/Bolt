#pragma once

#include "Vk_Assist.h"

namespace Bolt
{
	class Vk_Renderer;


	struct Raw_Shader
	{
		friend Vk_Renderer;

	private:
		static std::vector<char> Read_Byte_Code(const std::string& file_name);

	private:
		Shader_Modules m_shader_modules;
	};

	struct Shader
	{
		friend Vk_Renderer;

		enum class Defaults : uint8_t
		{
			Specular,
			Billboard,
			Diffuse,
		};

		#define SHADER_DEF_SPECULAR Bolt::Shader::Defaults::Specular
		#define SHADER_DEF_BILLBOARD Bolt::Shader::Defaults::Billboard
		#define SHADER_DEF_DIFFUSE Bolt::Shader::Defaults::Diffuse

		Shader() = default;

		Shader(Shader::Defaults default_shader) : _default(default_shader), custom(nullptr){}
		Shader(Raw_Shader* shader) : custom(shader) {}

		void Set_To_Default(Shader::Defaults default_shader)
		{
			_default = default_shader;
			custom = nullptr;
		}

		void Set_To_Custom(Raw_Shader* shader)
		{
			custom = shader;
		}

	private:
		Shader::Defaults _default = Shader::Defaults::Specular;
		Raw_Shader* custom = nullptr;
	};

	struct Image_Data
	{
		int32_t width = 0;
		int32_t height = 0;
		int32_t channels = 0;
		unsigned char* pixels = nullptr;
		void Free();

		bool Is_Transparent()
		{
			/*
			N=#comp     components
			1           grey
			2           grey, alpha
			3           red, green, blue
			4           red, green, blue, alpha
			*/

			return channels == 2 || channels == 4;
		}
	};

	struct Texture
	{
		friend Vk_Renderer;

	private:
		static Image_Data Load_Image(const char* file_path);

	private:
		Image_Description m_image;
		bool has_transparensy = false;
	};

	struct Material
	{
		friend Vk_Renderer;

		bool Has_Transparensy() { return has_transparency; }

	private:
		VkDescriptorSet m_descriptor_set = VK_NULL_HANDLE;
		VkPipeline m_pipeline = VK_NULL_HANDLE;

		Buffer_Description m_material_buffer;
		bool has_transparency = false;
	};

	struct Mesh
	{
		friend Vk_Renderer;

	private:
		Buffer_Description m_vertex_buffer;
		Buffer_Description m_index_buffer;
		uint32_t m_index_count = 0;
	};

	struct Render_Pass
	{
		friend Vk_Renderer;

	private:
		VkRenderPass renderpass = VK_NULL_HANDLE;
	};


	struct Scene_Descriptor
	{
		friend Vk_Renderer;
		
		void Set_Data(Camera_Data& camera_data, Enviroment_Data& enviroment_data)
		{
			this->camera_data = camera_data;
			this->enviroment_data = enviroment_data;
		}

	private:
		void Update(u32 index);
		void Write_Default_Values_Into_All_Descriptors();

		Camera_Data camera_data;
		Enviroment_Data enviroment_data;

		struct PFD
		{
			Buffer_Description buffer;
			VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
		};
		std::array<PFD, MAX_FRAMES_IN_FLIGHT> pfd;

		VkDevice device = VK_NULL_HANDLE;
	};
}

