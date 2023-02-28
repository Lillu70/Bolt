#include "Layer.h"
#include "Maths.h"

void Bolt::Layer::Update(Time time_step)
{
	User_Update(time_step);

	for (auto transfrom : m_components.Components<Transform_Glue>())
		transfrom->Glue();
}

void Bolt::Layer::Inject_Render_Submissions()
{
	Render_Submissions& submissions = *m_render_submissions;

	glm::vec3 camera_position;
	Update_Scene_Descriptor(submissions, camera_position);

	for (auto renderer : m_components.Components<Mesh_Renderer>())
	{
		if (submissions.Get_Active_Subpass_Index() != renderer->subpass_index)
			submissions.Set_Active_Subpass(renderer->subpass_index);
		
		f32 sqrd_distance_to_camera = renderer->material->Has_Transparensy() ? Maths::Distance_Squered(camera_position, renderer->transform.Position()) : 0;
		submissions.Submit_3D_Model(renderer->mesh, renderer->material, renderer->transform.Get_Matrix(), sqrd_distance_to_camera);
	}
	
	for (auto renderer : m_components.Components<Billboard_Renderer>())
	{
		if (submissions.Get_Active_Subpass_Index() != renderer->subpass_index)
			submissions.Set_Active_Subpass(renderer->subpass_index);
		
		f32 sqrd_distance_to_camera = renderer->material->Has_Transparensy() ? Maths::Distance_Squered(camera_position, renderer->transform.Position()) : 0;
		submissions.Submit_Billboard(renderer->mesh, renderer->material, renderer->transform.Position(), renderer->transform.Scale(), glm::vec3(0), sqrd_distance_to_camera);
	}

	User_Renderer_Submit(submissions, camera_position);
}

void Bolt::Layer::Update_Scene_Descriptor(Bolt::Render_Submissions& submissions, glm::vec3& out_camera_position)
{
	if (!submissions.Active_Pass().info.scene_descriptor) return;

	Entity data_holder = m_entity_spawner.Create(submissions.Active_Pass().info.data_id);
	Camera& camera = data_holder.Get<Camera>();
	out_camera_position = camera.transform.Position();

	Camera_Data cam_data;
	cam_data.view = glm::lookAt(out_camera_position, out_camera_position - glm::normalize(camera.transform.Orientation()), camera.up_direction);

	if (camera.projection_mode == Camera::Projection::Perspective)
		cam_data.proj = glm::perspective(glm::radians(camera.field_of_view), (float)Window_Extent().x / (float)Window_Extent().y, 0.1f, camera.far_clip_distance);
		
	else
	{
		f32 left, right, top, bottom;
		left = -(f32)Window_Extent().x * (camera.field_of_view / (camera.field_of_view * camera.field_of_view));
		right = +(f32)Window_Extent().x * (camera.field_of_view / (camera.field_of_view * camera.field_of_view));
		top = (f32)Window_Extent().y * (camera.field_of_view / (camera.field_of_view * camera.field_of_view));
		bottom = -(f32)Window_Extent().y * (camera.field_of_view / (camera.field_of_view * camera.field_of_view));
		cam_data.proj = glm::ortho(left, right, bottom, top, -camera.far_clip_distance, camera.far_clip_distance);
	}
	cam_data.proj[1][1] *= -1;

	Enviroment_Data& env_data = data_holder.Get<Enviroment_Data>();
	
	submissions.Active_Pass().info.scene_descriptor->Set_Data(cam_data, env_data);
}

Bolt::Entity_ID Bolt::Layer::Create_Camera_And_Env_Data_Entity()
{
	Entity ld = m_entity_spawner.Create();
	ld.Attach<Transform>().Set_Local_Orientation(glm::vec3(0,0,1));
	ld.Attach<Camera>(ld);
	ld.Attach<Name_Tag>(m_components, "Camera");
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
	Transform* camera_transform = &camera_entity.Get<Transform>();

	Entity skybox = Create_Entity();
	Transform& skybox_transform = skybox.Attach<Transform>();
	skybox_transform.Set_Local_Position(camera_transform->Position());
	
	skybox.Attach<Transform_Glue>(camera_transform, &skybox_transform);

	Mesh_Renderer_Create_Info create_info;
	create_info.root = &skybox_transform;
	create_info.render_pass = 0;
	create_info.scale = scale;
	create_info.shader = Shader(SHADER_DEF_DIFFUSE);

	Create_Mesh_Renderers_From_Model(model_path, create_info);
}

void Bolt::Layer::Create_Mesh_Renderers_From_Model(const std::string& model_path, Mesh_Renderer_Create_Info create_info)
{
	auto& mesh_and_material_names = Asset().Load_Model_File(model_path, create_info.shader, create_info.offset_by_origin);

	for (auto& [mesh_name, material_name] : mesh_and_material_names)
	{

		glm::vec3 position = create_info.position;
		glm::vec3 mesh_origin = Asset().Mesh_Origin(mesh_name);
			if (create_info.offset_by_origin)
				position += mesh_origin; 
		
			Entity entity = Create_Entity();
		entity.Attach<Transform>(create_info.root, position, create_info.rotation, create_info.scale);
		entity.Attach<Mesh_Renderer>(entity, Asset().Mesh(mesh_name), Asset().Material(material_name), create_info.render_pass);
		if (create_info.include_name_tag)
			entity.Attach<Name_Tag>(m_components, mesh_name);
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

void Bolt::Layer::Spawn_Circles_From_Model(const std::string& model_path, Transform* root, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, u32 render_pass)
{
	render_pass = 1;

	auto& mesh_and_material_names = Asset().Load_Model_File(model_path);
	Material_Properties mp;
	mp.transparensy = 0.5f;
	Material* material = Asset().Create_Material("circle_mat", mp, Asset().Texture("white.png"), Shader(SHADER_DEF_CIRCLE));

	for (auto& [mesh_name, material_name] : mesh_and_material_names)
	{
		Entity entity = Create_Entity();
		entity.Attach<Transform>(root, position, rotation, scale * 0.5f);
		entity.Attach<Billboard_Renderer>(entity, Asset().Mesh(mesh_name), material, render_pass);
	}
}

void Bolt::Layer::Push_Rendering_Subpass(Render_Pass_Info pass_info)
{
	m_render_submissions->Push_New_Subpass(pass_info);
}
