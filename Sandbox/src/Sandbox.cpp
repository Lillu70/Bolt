#include "Sandbox.h"
#include <iostream>

void Sandbox::User_Initialize(Bolt::App_Init& init_params)
{
	init_params.clear_color = glm::vec3(0.01f, 0.01f, 0.01f);


	Asset().Create_Material("gun", { 32.f }, Asset().Texture("UZI_Base_color.png"));
	Asset().Create_Material("default", { 1000.f }, Asset().Texture("white.png"));
	Asset().Create_Material("statue", { 1000.f }, Asset().Texture("texture.jpg"));

	

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("uzi.obj");
		render_obj.material = Asset().Material("gun");
		render_obj.transform = glm::translate(glm::mat4(1), glm::vec3(5.0f, 2.0f, -20.0f)) * glm::scale(glm::vec3(50));
	}
	
	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("Skull.obj");
		render_obj.material = Asset().Material("default");
		render_obj.transform = glm::translate(glm::mat4(1), glm::vec3(0.0f, 2.0f, -50.0f));
	}

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("bottle.obj");
		render_obj.material = Asset().Material("default");
		render_obj.transform = glm::translate(glm::mat4(1), glm::vec3(2.0f, 2.0f, -30.0f));
	}

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("bottle.obj");
		render_obj.material = Asset().Material("statue");
		render_obj.transform = glm::translate(glm::mat4(1), glm::vec3(-6.0f, 2.0f, -30.0f));
	}

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("ball.obj");
		render_obj.material = Asset().Material("default");
		render_obj.transform = glm::translate(glm::mat4(1), glm::vec3(-5.0f, 2.0f, -10.0f)) * glm::scale(glm::vec3(2));
	}

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("ball.obj");
		render_obj.material = Asset().Material("default");
		render_obj.transform = glm::translate(glm::mat4(1), glm::vec3(2.0f, 2.0f, -10.0f));
	}

	{
		Bolt::Render_Object& render_obj = m_render_objects.emplace_back();

		render_obj.mesh = Asset().Mesh("cube.obj");
		render_obj.material = Asset().Material("default");
		render_obj.transform = glm::mat4(1);
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

