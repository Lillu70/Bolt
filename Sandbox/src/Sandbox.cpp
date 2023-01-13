#include "Sandbox.h"
#include <iostream>

void Sandbox::User_Initialize(Bolt::App_Init& init_params)
{
	init_params.clear_color = glm::vec3(0.01f, 0.01f, 0.01f);
	
	Asset().Create_Material("spider_bot", { 500.f }, Asset().Texture("spidey_Baked_Base_Color.png"));
	Asset().Create_Material("ruin_wall", { 50.f }, Asset().Texture("Rock15Color.png"));
	
	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("spiedy_sfabblend.obj");
		render_obj.material = Asset().Material("spider_bot");
		render_obj.transform = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -10.f)) * glm::scale(glm::vec3(1));
	}

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("spiedy_sfabblend.obj");
		render_obj.material = Asset().Material("spider_bot");

		glm::mat4 tranform = glm::mat4(1);
		tranform *=	glm::translate(tranform, glm::vec3(-5.f, 2.f, -3.f));
		tranform *= glm::rotate(tranform, -90.f, glm::vec3(0.f, 1.f, 0.f));
		tranform *= glm::scale(glm::vec3(3.f));
		render_obj.transform = tranform;
	}

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("ruin_wall.obj");
		render_obj.material = Asset().Material("ruin_wall");
		render_obj.transform = glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(0.f, -3.f, -20.f)), -90.f, glm::vec3(0.f, 1.f, 0.f));
	}
}


void Sandbox::User_Update(Bolt::Time time_step)
{
	m_light_direction = glm::rotate(m_light_direction, m_light_rotation_speed * time_step, glm::vec3(0, 1, 0));

	Set_Global_Light_Source_Direction(m_light_direction);
	Set_Viewport_Matrix(m_camera.Update_Camera(time_step));
}


void Sandbox::User_Render(Bolt::Renderer* renderer)
{
	for (Bolt::Render_Object render_obj : m_render_objects)
		renderer->Submit_3D_Model(render_obj);
}

