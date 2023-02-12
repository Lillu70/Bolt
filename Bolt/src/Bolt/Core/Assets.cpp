#include "Assets.h"

Bolt::Assets::~Assets()
{
	for (auto& [file_path, mesh] : m_mesh_map)
		m_renderer_resource_factory->Destroy(mesh);

	for (auto& [file_path, texture] : m_texture_map)
		m_renderer_resource_factory->Destroy(texture);

	for (auto& [file_path, shader] : m_shader_map)
		m_renderer_resource_factory->Destroy(shader);

	for (auto& [file_path, material] : m_material_map)
		m_renderer_resource_factory->Destroy(material);

	for (auto& [key, render_pass] : m_render_pass_map)
		m_renderer_resource_factory->Destroy(render_pass);

	for (auto& [key, scene_descriptor] : m_scene_descriptor_map)
		m_renderer_resource_factory->Destroy(scene_descriptor);
} 

std::vector<std::pair<std::string, std::string>>& Bolt::Assets::Load_Model_File(const std::string& file_path, Bolt::Shader shader)
{
	m_active_shader = shader;

	std::string name, extension;
	Parsing::Extract_File_Extension_And_Name_From_CSTR(file_path.c_str(), extension, name);

	if (extension == "obj")
	{
		auto find = m_model_map.find(file_path);
		if (find != m_model_map.end())
			return find->second;

		m_model_map[file_path] = m_obj_loader.Load(m_mesh_path_prefix, name, this);
		return m_model_map[file_path];
	}
	else
		ERROR("Unsupported file format!");

	m_active_shader = {};
}

Bolt::Raw_Shader* Bolt::Assets::Shader(const std::string& name)
{
	auto find = m_shader_map.find(name);
	if (find != m_shader_map.end())
		return &find->second;

	ERROR("Shader not found!");
}

void Bolt::Assets::Create_Shader(const std::string& name, const std::string& frag_path, const std::string& vert_path)
{
	auto find = m_shader_map.find(name);
	if (find != m_shader_map.end())
		ERROR(std::string("Shader with name [") + name + "] already exists!");

	Bolt::Raw_Shader& shader = m_shader_map[name];
	std::string full_path_frag = m_shader_path_prefix + frag_path;
	std::string full_path_vert = m_shader_path_prefix + vert_path;
	m_renderer_resource_factory->Load_Shader(full_path_frag.c_str(), full_path_vert.c_str(), shader);
}

Bolt::Mesh* Bolt::Assets::Mesh(const std::string& mesh_name)
{
	auto find = m_mesh_map.find(mesh_name);
	if (find != m_mesh_map.end())
		return &find->second;

	ERROR("Mesh not found!");
}

Bolt::Texture* Bolt::Assets::Texture(const std::string& texture_path)
{
	auto find = m_texture_map.find(texture_path);
	if (find != m_texture_map.end())
		return &find->second;

	Bolt::Texture& texture = m_texture_map[texture_path];

	std::string full_path = m_texture_path_prefix + texture_path;
	m_renderer_resource_factory->Load_Texture(full_path.c_str(), texture);

	return &texture;
}

Bolt::Material* Bolt::Assets::Material(const std::string& material_name)
{
	auto find = m_material_map.find(material_name);
	if (find != m_material_map.end())
		return &find->second;

	WARN(std::string("Material with name [") + material_name + "] doesn't exists!");

	Create_Material(material_name, {}, Texture("white.png"));

	return Material(material_name);
}

Bolt::Scene_Descriptor* Bolt::Assets::Create_Scene_Descriptor(u32 main_pass_index, u32 subpass_index)
{
	u32 key = main_pass_index * 1000 + subpass_index;
	auto find = m_scene_descriptor_map.find(key);
	if (find != m_scene_descriptor_map.end())
		ERROR(std::string("Scene descripor with key [") + std::to_string(key) + "] already exists!");

	Bolt::Scene_Descriptor& scene_descriptor = m_scene_descriptor_map[key];

	m_renderer_resource_factory->Create_Scene_Descriptor(scene_descriptor);
	
	return &scene_descriptor;
}

Bolt::Render_Pass* Bolt::Assets::Create_Render_Pass(u32 main_pass_index, u32 subpass_index, Clear_Method color_clear, Clear_Method depth_clear)
{
	u32 key = main_pass_index * 1000 + subpass_index;
	auto find = m_render_pass_map.find(key);
	if (find != m_render_pass_map.end())
		ERROR(std::string("Render pass with key [") + std::to_string(key) + "] already exists!");

	Bolt::Render_Pass& render_pass = m_render_pass_map[key];

	m_renderer_resource_factory->Create_Render_Pass(render_pass, color_clear, depth_clear);

	return &render_pass;
}

void Bolt::Assets::Create_Material(const std::string& material_name, Material_Properties properties, Bolt::Texture* texture, Bolt::Shader shader)
{
	ASSERT(texture, "Texture is a nullptr");

	auto find = m_material_map.find(material_name);
	if (find != m_material_map.end())
		ERROR(std::string("Material with name [") + material_name + "] already exists!");

	Bolt::Material& material = m_material_map[material_name];
	m_renderer_resource_factory->Create_Material(properties, *texture, material, shader);
}

void Bolt::Assets::Push_Mesh(const std::string& name, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	ASSERT(!vertices.empty(), "Vertices vector is empty!");
	ASSERT(!indices.empty(), "Indices vector is empty!");

	auto find = m_mesh_map.find(name.c_str());
	if (find != m_mesh_map.end())
	{
		WARN("Mesh with the name " + name + " already exists");
		return;
	}

	Bolt::Mesh& mesh = m_mesh_map[name];
	m_renderer_resource_factory->Load_Mesh(vertices, indices, mesh);
}

void Bolt::Assets::Push_Material(const std::string& model_name, const std::string& material_name, Material_Data& material_data)
{
	auto find = m_material_map.find(material_name.c_str());
	if (find != m_material_map.end())
	{
		WARN("Material with the name " + material_name + " already exists");
		return;
	}

	Material_Properties properties;
	properties.roughness = material_data.specular_exponent;
	properties.diffuse_color = material_data.diffuse_color;
	properties.specular_color = material_data.specular_color;
	properties.transparensy = material_data.transparensy;

	if (material_data.m_deffuse_texture_name == "")
		material_data.m_deffuse_texture_name = "white.png";
	else
		material_data.m_deffuse_texture_name = model_name + "/" + material_data.m_deffuse_texture_name;
		

	Create_Material(material_name.c_str(), properties, Texture(material_data.m_deffuse_texture_name.c_str()), m_active_shader);
}