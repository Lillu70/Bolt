#pragma once
#include <Bolt.h>

class Sandbox : public Bolt::Application
{
public:
	Sandbox(){};
	~Sandbox() {};

protected:
	void User_Initialize(Bolt::App_Init& init_params) override;
	void User_Update(Bolt::Time time_step) override;

private:
	Bolt::Timer spawn_timer;

	glm::vec3 offset = glm::vec3(3, 0, 0.f);
	glm::vec3 last_pos = glm::vec3(0,0,-5.f);

	glm::vec3 m_light_direction = glm::vec3(3, 3, 3);
	float m_light_rotation_speed = 0.7f;
};