#include "Test_Layer.h"

void Test_Layer::Initialize()
{
	Create_Entity(player);
	
	player.Attach<Bolt::Transform>(glm::vec3(0,0,-5.f));
	Spawn_Entities_From_Model("Cube.obj", player.Find<Bolt::Transform>());
	Spawn_Billboards_From_Model("point_light.obj", player.Find<Bolt::Transform>(), glm::vec3(0,5,0));

	//Reference car!
	Spawn_Entities_From_Model("SPEEDSTERR.obj");
}

void Test_Layer::Update(Bolt::Time time_step)
{
	glm::vec3 movement_vector = glm::vec3(0);

	if (Input().Get(Bolt::Key::KP_8).Is_Pressed())
		movement_vector.y += 1;
	if (Input().Get(Bolt::Key::KP_2).Is_Pressed())
		movement_vector.y -= 1;
	if (Input().Get(Bolt::Key::KP_6).Is_Pressed())
		movement_vector.x += 1;
	if (Input().Get(Bolt::Key::KP_4).Is_Pressed())
		movement_vector.x -= 1;
	if (Input().Get(Bolt::Key::KP_5).Is_Pressed())
		movement_vector.z -= 1;
	if (Input().Get(Bolt::Key::KP_0).Is_Pressed())
		movement_vector.z += 1;
	if (Input().Get(Bolt::Key::KP_7).Is_Pressed())
	{
		movement_vector.y += 1;
		movement_vector.x -= 1;
	}
	if (Input().Get(Bolt::Key::KP_9).Is_Pressed())
	{
		movement_vector.y += 1;
		movement_vector.x += 1;
	}
	if (Input().Get(Bolt::Key::KP_1).Is_Pressed())
	{
		movement_vector.y -= 1;
		movement_vector.x -= 1;
	}
	if (Input().Get(Bolt::Key::KP_3).Is_Pressed())
	{
		movement_vector.y -= 1;
		movement_vector.x += 1;
	}
		
	if (movement_vector != glm::vec3(0))
	{
		float movement_speed = 10;

		movement_vector = glm::normalize(movement_vector);
		movement_vector *= movement_speed *= time_step.As_Seconds();

		player.Get<Bolt::Transform>().Offset_Local_Position(movement_vector);
	}

	glm::vec3 scale_vector = glm::vec3(0);

	if (Input().Get(Bolt::Key::KP_Devide).Is_Pressed())
		scale_vector -= 1.f;
	if (Input().Get(Bolt::Key::KP_Multiply).Is_Pressed())
		scale_vector += 1.f;

	if (scale_vector != glm::vec3(0))
	{
		float scale_speed = 1;
		scale_vector *= scale_speed *= time_step.As_Seconds();
		player.Get<Bolt::Transform>().Offset_Local_Scale(scale_vector);
	}
}
