#pragma once
#include <Bolt.h>
#include "Camera_Controller.h"

class Sandbox : public Bolt::Application
{
public:
	Sandbox() : m_camera(Input()) {};
	~Sandbox() {};

protected:
	void User_Initialize(Bolt::App_Init& init_params) override;
	void User_Update(Bolt::Time time_step) override;
	void User_Render(Bolt::Renderer* renderer) override;

private:
	Camera_Controller m_camera;
	Bolt::Mesh triangle;
	std::vector<Bolt::Render_Object> m_render_objects;

	glm::vec3 m_light_direction = glm::vec3(3, 3, 3);
	float m_light_rotation_speed = 0.7f;
};