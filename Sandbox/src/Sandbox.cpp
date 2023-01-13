#include "Sandbox.h"
#include <iostream>

void Sandbox::User_Initialize(Bolt::App_Init& init_params)
{
	init_params.clear_color = glm::vec3(0.01f, 0.01f, 0.01f);

	Asset().Create_Material("default", { 1000.f }, Asset().Texture("white.png"));
	Asset().Create_Material("chair", { 50.f }, Asset().Texture("chair.png"));
	

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("chair.obj");
		render_obj.material = Asset().Material("chair");
		render_obj.transform = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -10.f)) * glm::scale(glm::vec3(1));
	}

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("chair.obj");
		render_obj.material = Asset().Material("chair");
		render_obj.transform = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -20.f)) * glm::scale(glm::vec3(0.5f, 3.f, 2.f));
	}


	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("ball.obj");
		render_obj.material = Asset().Material("default");
		render_obj.transform = glm::translate(glm::mat4(1.f), glm::vec3(5.f, 0.f, -10.f)) * glm::scale(glm::vec3(1.f));
	}

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("ball.obj");
		render_obj.material = Asset().Material("default");
		render_obj.transform = glm::translate(glm::mat4(1.f), glm::vec3(10.f, 0.f, -10.f)) * glm::scale(glm::vec3(2.f, 2.f, 1.f));
	}

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("cube.obj");
		render_obj.material = Asset().Material("default");
		render_obj.transform = glm::translate(glm::mat4(1.f), glm::vec3(-5.f, 0.f, -10.f)) * glm::scale(glm::vec3(1.f));
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

