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
	player.Attach<Bolt::Transform>();
	player.Attach<Bolt::Name_Tag>("Player");
	
	Bolt::Mesh_Renderer_Create_Info create_info;
	create_info.root = &player.Get<Bolt::Transform>();
	create_info.offset_by_origin = true;
	Create_Mesh_Renderers_From_Model("BotBolt.obj", create_info);

	Create_Skybox_From_Model("Skybox.obj", glm::vec3(1));
	
	create_info.root = nullptr;
	create_info.offset_by_origin = true;
	glm::vec3 pos = glm::vec3(0, 0, 3);
	for (int i = 0; i < 10; i++)
	{
		//Spawn_Entities_From_Model("SPEEDSTERR.obj", nullptr, pos);
		create_info.position = pos;
		
		Create_Mesh_Renderers_From_Model("SPEEDSTERR.obj", create_info);
		pos += glm::vec3(0, 0, 5);
	}
	

	camera = Get_Camera_Entity();
	camera.Attach<Bolt::Camera_Controller>(camera);
}

void Test_Layer::Update(Bolt::Time time_step)
{
	if (Input().Key_State(Bolt::Key::ESCAPE).Is_Released()) Close_App();
	
	Transform_Select_And_Modify(time_step.As_Seconds());

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

void Test_Layer::Display_Name_Tags()
{
	for(auto name_tag : ECS().Components<Bolt::Name_Tag>())
		std::cout << (name_tag->name) << "\n";
}

void Test_Layer::Transform_Select_And_Modify(float time_step)
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

	Bolt::Entity_ID id_of_selection = ECS().Get_Entity_ID(ECS().Components<Bolt::Transform>()[selected_index]);
	Bolt::Entity selected_entity = Create_Entity(id_of_selection);


	if (index_changed)
	{
		index_changed = false;
		std::cout << "Selected Index: " << selected_index << "\n";
		std::string name_of_selection;
		Bolt::Name_Tag* name_tag = selected_entity.Find<Bolt::Name_Tag>();
		if (name_tag)
			std::cout << "Name: " + name_tag->name + "\n";
		else
			std::cout << "Entity missing name tag!\n";

		std::cout << "\n";
	}

	glm::vec3 movement_vector = glm::vec3(0);


	//No modifiers. Do movement!
	if (Input().Key_State(Bolt::Key::LEFT_SHIFT).Negative())
	{
		glm::vec3 movement_vector = glm::vec3(0);
		if (Input().Key_State(Bolt::Key::KP_8).Positive())
			movement_vector = glm::vec3(0, 1, 0);
		if (Input().Key_State(Bolt::Key::KP_2).Positive())
			movement_vector = glm::vec3(0, -1, 0);
		if (Input().Key_State(Bolt::Key::KP_4).Positive())
			movement_vector = glm::vec3(-1, 0, 0);
		if (Input().Key_State(Bolt::Key::KP_6).Positive())
			movement_vector = glm::vec3(1, 0, 0);
		if (Input().Key_State(Bolt::Key::KP_5).Positive())
			movement_vector = glm::vec3(0, 0, -1);
		if (Input().Key_State(Bolt::Key::KP_0).Positive())
			movement_vector = glm::vec3(0, 0, 1);

		if (movement_vector != glm::vec3(0))
			selected_entity.Get<Bolt::Transform>().Offset_Local_Position(glm::normalize(movement_vector) * time_step);
		
			
	}
	//Scaling.
	else if (Input().Key_State(Bolt::Key::LEFT_SHIFT).Positive())
	{
		

		glm::vec3 scaling_vector = glm::vec3(0);
		if (Input().Key_State(Bolt::Key::KP_8).Positive())
			scaling_vector = glm::vec3(0, 1, 0);
		if (Input().Key_State(Bolt::Key::KP_2).Positive())
			scaling_vector = glm::vec3(0, -1, 0);
		if (Input().Key_State(Bolt::Key::KP_4).Positive())
			scaling_vector = glm::vec3(-1, 0, 0);
		if (Input().Key_State(Bolt::Key::KP_6).Positive())
			scaling_vector = glm::vec3(1, 0, 0);
		if (Input().Key_State(Bolt::Key::KP_5).Positive())
			scaling_vector = glm::vec3(0, 0, -1);
		if (Input().Key_State(Bolt::Key::KP_0).Positive())
			scaling_vector = glm::vec3(0, 0, 1);

		if (scaling_vector != glm::vec3(0))
			selected_entity.Get<Bolt::Transform>().Offset_Local_Scale(glm::normalize(scaling_vector) * time_step);
		
			
	}





	//
	// if (movement_vector != glm::vec3(0))
	//	selected_entity.Get<Bolt::Transform>().Offset_Local_Position(glm::normalize(movement_vector) * time_step);
	//else if (Input().Get(Bolt::Key::Left_Shift).Is_Repeated())
	//{
	//glm::vec3 scale_vector = glm::vec3(0);
	//if (Input().Get(Bolt::Key::KP_8).Is_Pressed())
	//	scale_vector = glm::vec3(0, 1, 0);
	//if (Input().Get(Bolt::Key::KP_2).Is_Pressed())
	//	scale_vector = glm::vec3(0, -1, 0);
	//if (Input().Get(Bolt::Key::KP_4).Is_Pressed())
	//	scale_vector = glm::vec3(-1, 0, 0);
	//if (Input().Get(Bolt::Key::KP_6).Is_Pressed())
	//	scale_vector = glm::vec3(1, 0, 0);
	//
	//if (scale_vector != glm::vec3(0))
	//	selected_entity.Get<Bolt::Transform>().Offset_Local_Scale(glm::normalize(scale_vector) * time_step);

}
