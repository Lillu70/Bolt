#include "Test_Layer.h"
#include <Bolt/Core/Key_Codes.h>

void Test_Layer::Initialize()
{
	Bolt::Render_Pass_Info rps;
	rps.render_pass = Asset().Create_Render_Pass(Layer_Index(), Subpass_Count() + 1, Bolt::Clear_Method::Load, Bolt::Clear_Method::Clear);
	Push_Rendering_Subpass(rps);
	
	rps.render_pass = Asset().Create_Render_Pass(Layer_Index(), Subpass_Count() + 1, Bolt::Clear_Method::Load, Bolt::Clear_Method::Clear);
	Push_Rendering_Subpass(rps);


	player = Create_Entity();
	player.Attach<Bolt::Transform>().Set_Local_Position(glm::vec3(0,0,-5));
	player.Attach<Bolt::Name_Tag>(ECS(), "Player");
	
	Bolt::Mesh_Renderer_Create_Info create_info;
	create_info.root = &player.Get<Bolt::Transform>();
	create_info.offset_by_origin = true;
	Create_Mesh_Renderers_From_Model("BotBolt.obj", create_info);

	Create_Skybox_From_Model("Skybox.obj", glm::vec3(1));
	
	
	create_info.offset_by_origin = true;
	glm::vec3 pos = glm::vec3(0, 0, 3);
	for (int i = 0; i < 10; i++)
	{
		Bolt::Entity car = Create_Entity();
		car.Attach<Bolt::Name_Tag>(ECS(), "Car");
		car.Attach<Bolt::Transform>().Set_Local_Position(pos);

		create_info.root = &car.Get<Bolt::Transform>();

		//create_info.position = pos;	
		Create_Mesh_Renderers_From_Model("SPEEDSTERR.obj", create_info);
		pos += glm::vec3(0, 0, 5);
	}
	

	camera = Get_Camera_Entity();
	camera.Attach<Bolt::Camera_Controller>(camera);
}

void Test_Layer::User_Update(Bolt::Time time_step)
{
	if (Input().Key_State(Bolt::Key::ESCAPE).Is_Released()) Close_App();
	if (Input().Key_State(Bolt::Key::T).Is_Released()) m_visualize_transforms = !m_visualize_transforms;
	
	Select_And_Modify_Transforms(time_step.As_Seconds());

	glm::vec3 movement = glm::vec3(0);

	if (Input().Key_State(Bolt::Key::LEFT_SHIFT).Positive())
	{
		if (Input().Key_State(Bolt::Key::W).Positive())
			movement += glm::vec3(0, 0, -1);
		if (Input().Key_State(Bolt::Key::S).Positive())
			movement += glm::vec3(0, 0, 1);
	}
	else
	{
		if (Input().Key_State(Bolt::Key::W).Positive())
			movement += glm::vec3(0, -1, 0);
		if (Input().Key_State(Bolt::Key::S).Positive())
			movement += glm::vec3(0, 1, 0);
	}
	if (Input().Key_State(Bolt::Key::A).Positive())
		movement += glm::vec3(1, 0, 0);
	if (Input().Key_State(Bolt::Key::D).Positive())
		movement += glm::vec3(-1, 0, 0);

	float yaw = 0;
	float pich = 0;
	if (Input().Key_State(Bolt::Key::RIGHT).Positive())
		yaw += 1;
	if (Input().Key_State(Bolt::Key::LEFT).Positive())
		yaw -= 1;
	if (Input().Key_State(Bolt::Key::DOWN).Positive())
		pich += 1;
	if (Input().Key_State(Bolt::Key::UP).Positive())
		pich -= 1;
	
	Bolt::Camera_Controller& cam = camera.Get<Bolt::Camera_Controller>();
	cam.Relative_Move(movement, 3 * time_step.As_Seconds());
	cam.Rotate(yaw, pich, 100 * time_step);

	camera.Get<Bolt::Enviroment_Data>().global_light_direction = glm::rotate(camera.Get<Bolt::Enviroment_Data>().global_light_direction, time_step.As_Seconds(), glm::vec3(0, 1, 0));
}

void Test_Layer::User_Renderer_Submit(Bolt::Render_Submissions& submissions, glm::vec3 camera_position)
{
	if (!m_visualize_transforms) return;

	auto& mesh_and_material_names = Asset().Load_Model_File("plane.obj");
	Bolt::Material_Properties mp;
	mp.transparensy = 0.5f;
	Bolt::Material* material = Asset().Create_Material("circle_mat", mp, Asset().Texture("white.png"), Bolt::Shader(SHADER_DEF_CIRCLE));
	Bolt::Mesh* mesh = Asset().Mesh(mesh_and_material_names[0].first);

	submissions.Set_Active_Subpass(-1);

	std::vector<Bolt::Transform*> all_transforms = ECS().Components<Bolt::Transform>();
	Bolt::Transform& seleceted_transform = m_selected_entity.Get<Bolt::Transform>();

	
	Bolt::Util::Vector_Find_Swap_Top_An_Pop(all_transforms, &seleceted_transform);

	std::vector<Bolt::Transform*> selected_children = seleceted_transform.Build_Downwards_Hierarchy();

	for (Bolt::Transform* child : selected_children)
		Bolt::Util::Vector_Find_Swap_Top_An_Pop(all_transforms, child);
	
	//Draw selected transform.
	submissions.Submit_Billboard(mesh, material, seleceted_transform.Position(), 
		glm::vec3(0.2f), glm::vec3(0.1, 8, 0.1), Bolt::Maths::Distance_Squered(camera_position, seleceted_transform.Position()));

	//Draw selected transforms children.
	for (Bolt::Transform* transform : selected_children)
	{
		glm::vec3 scale(0.15f);
		glm::vec3 color(0.0, 0.5, 0.5);

		glm::vec3 pos = transform->Position();
		Bolt::f32 sqrd_distance_to_camera = Bolt::Maths::Distance_Squered(camera_position, pos);
		submissions.Submit_Billboard(mesh, material, pos, scale, color, sqrd_distance_to_camera);
	}

	//Draw un-selected transforms.
	for (Bolt::Transform* transform : all_transforms)
	{
		glm::vec3 scale(0.1f);
		glm::vec3 color(0.33, 0, 0.33);

		glm::vec3 pos = transform->Position();
		Bolt::f32 sqrd_distance_to_camera = Bolt::Maths::Distance_Squered(camera_position, pos);
		submissions.Submit_Billboard(mesh, material, pos, scale, color, sqrd_distance_to_camera);
	}
}

void Test_Layer::Select_And_Modify_Transforms(float time_step)
{
	static Bolt::i32 selected_index = 0;
	static bool index_changed = true;

	if (Input().Key_State(Bolt::Key::Y).Is_Pressed())
	{
		selected_index++;
		if (selected_index == Bolt::u32(ECS().Components<Bolt::Transform>().size()))
			selected_index = 0;
		index_changed = true;
	}

	if (Input().Key_State(Bolt::Key::H).Is_Pressed())
	{
		selected_index--;
		if (selected_index == std::numeric_limits<Bolt::u32>::max())
			selected_index = Bolt::u32(ECS().Components<Bolt::Transform>().size()) - 1;
		index_changed = true;
	}

	if (index_changed)
	{
		index_changed = false;
		Bolt::Entity_ID id_of_selection = ECS().Get_Entity_ID(ECS().Components<Bolt::Transform>()[selected_index]);
		m_selected_entity = Create_Entity(id_of_selection);
		std::cout << "Selected Index: " << selected_index << "\n";
		std::string name_of_selection;
		Bolt::Name_Tag* name_tag = m_selected_entity.Find<Bolt::Name_Tag>();
		if (name_tag)
			std::cout << "Name: " + name_tag->Get_Unique() + "\n";
		else
			std::cout << "Entity is not named.\n";

		std::cout << "\n";
	}

	glm::vec3 movement_vector = glm::vec3(0);


	//No modifiers. Do movement!
	if (Input().Key_State(Bolt::Key::LEFT_SHIFT).Negative() && Input().Key_State(Bolt::Key::LEFT_ALT).Negative())
	{
		glm::vec3 movement_vector;
		if ( Get_KP_Directional_Input(movement_vector) )
			m_selected_entity.Get<Bolt::Transform>().Offset_Local_Position_Relative_To_Orientation(movement_vector * time_step);
	}
	else if (Input().Key_State(Bolt::Key::LEFT_SHIFT).Positive())
	{
		glm::vec3 scaling_vector;
		if ( Get_KP_Directional_Input(scaling_vector) )
			m_selected_entity.Get<Bolt::Transform>().Offset_Local_Scale(glm::normalize(scaling_vector) * time_step);
	}
	else if (Input().Key_State(Bolt::Key::LEFT_ALT).Positive())
	{
		glm::vec3 rotation_vector;
		if ( Get_KP_Directional_Input(rotation_vector) )
			m_selected_entity.Get<Bolt::Transform>().Offset_Local_Orientation(glm::normalize(rotation_vector) * time_step);
	}

	if (Input().Key_State(Bolt::Key::P).Is_Pressed())
		m_selected_entity.Get<Bolt::Transform>().Explode();
}

bool Test_Layer::Get_KP_Directional_Input(glm::vec3& direction)
{
	direction = glm::vec3(0);
	if (Input().Key_State(Bolt::Key::KP_8).Positive())
		direction = glm::vec3(0, 1, 0);
	if (Input().Key_State(Bolt::Key::KP_2).Positive())
		direction = glm::vec3(0, -1, 0);
	if (Input().Key_State(Bolt::Key::KP_4).Positive())
		direction = glm::vec3(1, 0, 0);
	if (Input().Key_State(Bolt::Key::KP_6).Positive())
		direction = glm::vec3(-1, 0, 0);
	if (Input().Key_State(Bolt::Key::KP_5).Positive())
		direction = glm::vec3(0, 0, 1);
	if (Input().Key_State(Bolt::Key::KP_0).Positive())
		direction = glm::vec3(0, 0, -1);

	return direction != glm::vec3(0);
}

void Test_Layer::Extract_Selected_Transforms_From_Hierarhy(const std::vector<Bolt::Transform*>& selected_child_hierarchy, Bolt::Transform* selected_transform, std::vector<Bolt::Transform*>& hierarchy)
{
	//Discard the selected entity
	
}
