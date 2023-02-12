#include "Sandbox.h"
#include "Test_Layer.h"

void Sandbox::User_Initialize(Bolt::App_Init& init_params)
{
	init_params.clear_color = glm::vec3(0.01f, 0.01f, 0.01f);
	Push_Layer(std::make_unique<Test_Layer>());
}

void Sandbox::User_Update(Bolt::Time time_step)
{
	m_light_direction = glm::rotate(m_light_direction, m_light_rotation_speed * time_step, glm::vec3(0, 1, 0));
}