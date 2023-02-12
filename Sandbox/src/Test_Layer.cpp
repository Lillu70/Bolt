#include "Test_Layer.h"

void Test_Layer::Initialize()
{
	Bolt::Render_Pass_Info rps;
	rps.render_pass = Asset().Create_Render_Pass(Layer_Index(), Subpass_Count() + 1, Bolt::Clear_Method::Load, Bolt::Clear_Method::Clear);
	Push_Rendering_Subpass(rps);
	
	//Reference car!
	glm::vec3 pos = glm::vec3(0);
	for (int i = 0; i < 20; i++)
	{
		Bolt::Entity ent = Create_Entity();

		ent.Attach<Bolt::Transform>(pos + glm::vec3(0, 0, -5.f));
		Spawn_Entities_From_Model("Cube.obj", ent.Find<Bolt::Transform>());
		Spawn_Billboards_From_Model("point_light.obj", ent.Find<Bolt::Transform>(), glm::vec3(0, 5, 0));
		Spawn_Billboards_From_Model("point_light.obj", ent.Find<Bolt::Transform>(), glm::vec3(0, 10, 0));
		Spawn_Billboards_From_Model("point_light.obj", ent.Find<Bolt::Transform>(), glm::vec3(0, 15, 0));

		Spawn_Entities_From_Model("SPEEDSTERR.obj", nullptr, pos);
		pos += glm::vec3(3, 0, 0);
	}
	Create_Skybox_From_Model("Skybox.obj", glm::vec3(1));

	camera = Get_Camera_Entity();
	camera.Attach<Bolt::Camera_Controller>(camera);
}

void Test_Layer::Update(Bolt::Time time_step)
{
	glm::vec3 movement = glm::vec3(0);
	
	if (Input().Get(Bolt::Key::W).Is_Pressed())
		if(Input().Get(Bolt::Key::Left_Shift).Is_Pressed())
			movement.z -= 1;
		else
			movement.y -= 1;
	if (Input().Get(Bolt::Key::S).Is_Pressed())
		if (Input().Get(Bolt::Key::Left_Shift).Is_Pressed())
			movement.z += 1;
		else
			movement.y += 1;
	if (Input().Get(Bolt::Key::D).Is_Pressed())
		movement.x -= 1;
	if (Input().Get(Bolt::Key::A).Is_Pressed())
		movement.x += 1;

	float yaw = 0;
	float pich = 0;

	if (Input().Get(Bolt::Key::Right).Is_Pressed())
		yaw += 1;
	if (Input().Get(Bolt::Key::Left).Is_Pressed())
		yaw -= 1;

	if (Input().Get(Bolt::Key::Down).Is_Pressed())
		pich += 1;
	if (Input().Get(Bolt::Key::Up).Is_Pressed())
		pich -= 1;

	float horizontal_move = 0;
	if (Input().Get(Bolt::Key::G).Is_Pressed())
		horizontal_move += 1;

	float vertical_move = 0;
	if (Input().Get(Bolt::Key::G).Is_Pressed())
		vertical_move += 1;

	//camera.Get<Bolt::Camera_Controller>().Relative_Move_Vertical(vertical_move, 3 * time_step.As_Seconds());
	camera.Get<Bolt::Camera_Controller>().Relative_Move(movement, 3 * time_step.As_Seconds());
	camera.Get<Bolt::Camera_Controller>().Rotate(yaw, pich, 100 * time_step);

	camera.Get<Bolt::Enviroment_Data>().global_light_direction = glm::rotate(camera.Get<Bolt::Enviroment_Data>().global_light_direction, time_step.As_Seconds(), glm::vec3(0, 1, 0));

	
}
