#pragma once
#include "../Vulkan/Vk_Internal_Components.h"
#include "Assets_Resource_Injector.h"

#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include <set>


namespace Bolt
{
	
	
	namespace Parsing
	{
		void Extract_File_Extension_And_Name_From_CSTR(const char* cstr, std::string& extension, std::string& path);
		char Read_Until_Delim(std::ifstream& file, std::string& output, const char* delim, uint32_t delim_count = 1);
	}

	class Model_Loader
	{
	public:
		struct Mesh_Data
		{
			std::unordered_map<Vertex, uint32_t> unique_vertices;
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
		};

	public:
		//Returns names of the loaded meshes.
		virtual std::vector<std::pair<std::string, std::string>> Load(const std::string& sub_folder, const std::string& file_name, Assets_Resource_Injector* injector) = 0;
		
	protected:
		virtual const char* File_Suffix() = 0;
		virtual std::string Full_Path(const char* sub_folder, const char* file_name, const char* file_suffix) = 0;
	};

	class Obj_Loader : public Model_Loader
	{
	public:
		//Returns names of the loaded meshes paired with the material name.
		std::vector<std::pair<std::string, std::string>> Load(const std::string& sub_folder, const std::string& file_name, Assets_Resource_Injector* injector) override;

		void Push_Sub_Meshes(std::unordered_map<std::string, Model_Loader::Mesh_Data>& sub_meshes, std::vector<std::pair<std::string, std::string>>& shape_names, const std::string& active_object_name, Assets_Resource_Injector* injector);

		
	protected:
		const char* File_Suffix() override;
		std::string Full_Path(const char* sub_folder, const char* file_name, const char* file_suffix) override;
		void Load_mtl(const std::string& model_name, const std::string& file_name, Assets_Resource_Injector* injector);

	private:
		std::string Read_Position_Or_Normal_In_Tight_Loop(std::ifstream& file, const char* identifier, std::vector<glm::vec3>& ouput);
		std::string Read_Texture_Coordinate_In_Tight_Loop(std::ifstream& file, std::vector<glm::vec2>& ouput);
		std::string Read_Faces_In_Tight_Loop(std::ifstream& file, std::vector<glm::vec3>& positions, std::vector<glm::vec2>& texture_coord, std::vector<glm::vec3>& normals, Model_Loader::Mesh_Data& output_mesh);

		
		void Read_Vec3(std::ifstream& file, glm::vec3& vec3);
		void Read_Vec2(std::ifstream& file, glm::vec2& vec3);
		void Read_Face(std::ifstream& file, std::string& word, std::array<uint32_t, 3>& vertex_elements, uint32_t vert_member, uint32_t face_index);

	private:
		std::set<std::string> m_parsed_mtllib_files;

	};
}



