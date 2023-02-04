#include "Layer.h"


Bolt::Render_Submission& Bolt::Layer::Get_Render_Submissions()
{
	m_submissions.Clear();

	for (auto mesh_renderer : m_components.Components<Mesh_Renderer>())
	{
		auto position = mesh_renderer->transform.Local_Position();

		auto& model = m_submissions.m_models_3D.emplace_back();
		model.material = mesh_renderer->material;
		model.mesh = mesh_renderer->mesh;
		model.transform = mesh_renderer->transform.Get_Matrix();
	}

	for (auto billboard_renderer : m_components.Components<Billboard_Renderer>())
	{
		auto position = billboard_renderer->transform.Local_Position();

		auto& model = m_submissions.m_billboards.emplace_back();
		model.material = billboard_renderer->material;
		model.mesh = billboard_renderer->mesh;
		model.position = billboard_renderer->Position();
		model.scale = billboard_renderer->Scale();
	}

	return m_submissions;
}

Bolt::Entity Bolt::Layer::Create_Entity()
{
	return m_entity_spawner.Create();
}

void Bolt::Layer::Create_Entity(Entity& entity)
{
	m_entity_spawner.Create(entity);
}

void Bolt::Layer::Spawn_Entities_From_Model(const std::string& model_path, Transform* root, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	auto& mesh_and_material_names = Asset().Load_Model_File(model_path);

	for (auto& [mesh_name, material_name] : mesh_and_material_names)
	{
		Entity entity = Create_Entity();
		entity.Attach<Transform>(root, position, rotation, scale);
		entity.Attach<Mesh_Renderer>(entity, Asset().Mesh(mesh_name), Asset().Material(material_name));
	}
}

void Bolt::Layer::Spawn_Billboards_From_Model(const std::string& model_path, Transform* root, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	auto& mesh_and_material_names = Asset().Load_Model_File(model_path, Shader(SHADER_DEF_BILLBOARD));

	for (auto& [mesh_name, material_name] : mesh_and_material_names)
	{
		Entity entity = Create_Entity();
		entity.Attach<Transform>(root, position, rotation, scale);
		entity.Attach<Billboard_Renderer>(entity, Asset().Mesh(mesh_name), Asset().Material(material_name));
	}
}
