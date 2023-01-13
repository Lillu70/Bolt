#pragma once
#include <Bolt.h>
#include "Camera_Controller.h"

class Sandbox : public Bolt::Application
{
public:
	Sandbox() : camera(Input()) {};
	~Sandbox() {};

protected:
	void User_Initialize(Bolt::App_Init& init_params) override;
	void User_Update(Bolt::Time time_step) override;
	void User_Render(Bolt::Renderer* renderer) override;
	void User_App_Exit() override;

private:
	Camera_Controller camera;

	Bolt::Render_Object uzi;
	Bolt::Render_Object skull_table;
	Bolt::Render_Object bottle_1;
	Bolt::Render_Object bottle_2;
	Bolt::Render_Object ball_1;
	Bolt::Render_Object ball_2;


	glm::vec3 light_direction = glm::vec3(3, 3, 3);
	float light_rotation_speed = 0.5f;
};