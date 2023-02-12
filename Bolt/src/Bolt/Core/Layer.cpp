#include "Layer.h"


void Bolt::Layer::Inject_Render_Submissions()
{
	Render_Submissions& submissions = *m_render_submissions;

	Update_Scene_Descriptor(submissions);

	for (auto mesh_renderer : m_components.Components<Mesh_Renderer>())
	{
		if (submissions.Get_Active_Subpass_Index() != mesh_renderer->subpass_index)
			submissions.Set_Active_Subpass(mesh_renderer->subpass_index);
		
		submissions.Submit_3D_Model(mesh_renderer->mesh, mesh_renderer->material, mesh_renderer->transform.Get_Matrix());
	}
	
	for (auto billboard_renderer : m_components.Components<Billboard_Renderer>())
	{
		if (submissions.Get_Active_Subpass_Index() != billboard_renderer->subpass_index)
			submissions.Set_Active_Subpass(billboard_renderer->subpass_index);
		
		submissions.Submit_Billboard(billboard_renderer->mesh, billboard_renderer->material, billboard_renderer->transform.Position(), billboard_renderer->transform.Scale());
	}	
}

void Bolt::Layer::Update_Scene_Descriptor(Bolt::Render_Submissions& submissions)
{
	if (!submissions.Active_Pass().info.scene_descriptor) return;

	Entity data_holder = m_entity_spawner.Create(submissions.Active_Pass().info.data_id);
	Camera& camera = data_holder.Get<Camera>();

	Camera_Data cam_data;
	cam_data.pos = glm::translate(glm::mat4(1), camera.transform.Position());
	cam_data.view = glm::lookAt(camera.transform.Position(), camera.transform.Position() - glm::normalize(camera.transform.Rotation()), camera.up_direction);
	//cam_data.view = glm::lookAt(camera.transform.Position() - glm::normalize(camera.transform.Rotation()), camera.transform.Position(), camera.up_direction);
	//cam_data.view = glm::translate(glm::mat4(1), camera.transform.Position());
	cam_data.proj = glm::perspective(glm::radians(75.f), (float)Window_Extent().x / (float)Window_Extent().y, 0.1f, 1000.0f);
	cam_data.proj[1][1] *= -1;
	
	Enviroment_Data& env_data = data_holder.Get<Enviroment_Data>();
	
	submissions.Active_Pass().info.scene_descriptor->Set_Data(cam_data, env_data);
}

Bolt::Entity_ID Bolt::Layer::Create_Camera_And_Env_Data_Entity()
{
	Entity ld = m_entity_spawner.Create();
	ld.Attach<Transform>().Set_Local_Rotation(glm::vec3(0,0,1));
	ld.Attach<Camera>(ld);
	ld.Attach<Enviroment_Data>();

	return ld.ID();
}

Bolt::Entity Bolt::Layer::Create_Entity()
{
	return m_entity_spawner.Create();
}

Bolt::Entity Bolt::Layer::Create_Entity(Entity_ID id)
{
	return m_entity_spawner.Create(id);
}

Bolt::Entity Bolt::Layer::Get_Camera_Entity(u32 index)
{
	auto& cameras = m_components.Components<Camera>();
	ASSERT(index < cameras.size(), "Array out of bounds!");
	return Create_Entity(m_components.Get_Entity_ID(cameras[index]));
}

void Bolt::Layer::Create_Entity(Entity& entity)
{
	m_entity_spawner.Create(entity);
}

glm::uvec2 Bolt::Layer::Window_Extent()
{
	return glm::uvec2(m_extent_ptr->width, m_extent_ptr->height);
}

void Bolt::Layer::Create_Skybox_From_Model(const std::string& model_path, glm::vec3 scale)
{
	Entity camera_entity = Get_Camera_Entity();

	Mesh_Renderer_Create_Info create_info;
	create_info.root = &camera_entity.Get<Transform>();
	create_info.render_pass = 0;
	create_info.scale = scale;
	create_info.shader = Shader(SHADER_DEF_DIFFUSE);

	Create_Mesh_Renderers_From_Model(model_path, create_info);
}

void Bolt::Layer::Create_Mesh_Renderers_From_Model(const std::string& model_path, Mesh_Renderer_Create_Info create_info)
{
	auto& mesh_and_material_names = Asset().Load_Model_File(model_path, create_info.shader);

	for (auto& [mesh_name, material_name] : mesh_and_material_names)
	{
		Entity entity = Create_Entity();
		entity.Attach<Transform>(create_info.root, create_info.position, create_info.rotation, create_info.scale);
		entity.Attach<Mesh_Renderer>(entity, Asset().Mesh(mesh_name), Asset().Material(material_name), create_info.render_pass);
	}
}

void Bolt::Layer::Spawn_Entities_From_Model(const std::string& model_path, Transform* root, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, u32 render_pass)
{
	auto& mesh_and_material_names = Asset().Load_Model_File(model_path);

	for (auto& [mesh_name, material_name] : mesh_and_material_names)
	{
		Entity entity = Create_Entity();
		entity.Attach<Transform>(root, position, rotation, scale);
		entity.Attach<Mesh_Renderer>(entity, Asset().Mesh(mesh_name), Asset().Material(material_name), render_pass);
	}
}

void Bolt::Layer::Spawn_Billboards_From_Model(const std::string& model_path, Transform* root, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, u32 render_pass)
{
	auto& mesh_and_material_names = Asset().Load_Model_File(model_path, Shader(SHADER_DEF_BILLBOARD));

	for (auto& [mesh_name, material_name] : mesh_and_material_names)
	{
		Entity entity = Create_Entity();
		entity.Attach<Transform>(root, position, rotation, scale);
		entity.Attach<Billboard_Renderer>(entity, Asset().Mesh(mesh_name), Asset().Material(material_name), render_pass);
	}
}

void Bolt::Layer::Push_Rendering_Subpass(Render_Pass_Info pass_info)
{
	m_render_submissions->Push_New_Subpass(pass_info);
}
