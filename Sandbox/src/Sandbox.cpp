#include "Sandbox.h"
#include <iostream>

void Sandbox::User_Initialize(Bolt::App_Init& init_params)
{
	init_params.clear_color = glm::vec3(0.01f, 0.01f, 0.01f);


	Asset().Create_Material("gun", { 32.f }, Asset().Texture("UZI_Base_color.png"));
	Asset().Create_Material("default", { 1000.f }, Asset().Texture("white.png"));
	Asset().Create_Material("statue", { 1000.f }, Asset().Texture("texture.jpg"));

	{
		uzi.mesh = Asset().Mesh("uzi.obj");
		uzi.material = Asset().Material("gun");
		uzi.transform = glm::translate(glm::mat4(1), glm::vec3(5.0f, 2.0f, -20.0f)) * glm::scale(glm::vec3(50));
	}
	
	{
		skull_table.mesh = Asset().Mesh("Skull.obj");
		skull_table.material = Asset().Material("default");
		skull_table.transform = glm::translate(glm::mat4(1), glm::vec3(0.0f, 2.0f, -50.0f));
	}

	{
		bottle_1.mesh = Asset().Mesh("bottle.obj");
		bottle_1.material = Asset().Material("default");
		bottle_1.transform = glm::translate(glm::mat4(1), glm::vec3(2.0f, 2.0f, -30.0f));
	}

	{
		bottle_2.mesh = Asset().Mesh("bottle.obj");
		bottle_2.material = Asset().Material("statue");
		bottle_2.transform = glm::translate(glm::mat4(1), glm::vec3(-6.0f, 2.0f, -30.0f));
	}

	{
		ball_1.mesh = Asset().Mesh("ball.obj");
		ball_1.material = Asset().Material("default");
		ball_1.transform = glm::translate(glm::mat4(1), glm::vec3(-5.0f, 2.0f, -10.0f)) * glm::scale(glm::vec3(2));
	}

	{
		ball_2.mesh = Asset().Mesh("ball.obj");
		ball_2.material = Asset().Material("default");
		ball_2.transform = glm::translate(glm::mat4(1), glm::vec3(2.0f, 2.0f, -10.0f));
	}

}


void Sandbox::User_Update(Bolt::Time time_step)
{
	light_direction = glm::rotate(light_direction, light_rotation_speed * time_step, glm::vec3(0, 1, 0));

	Set_Global_Light_Source_Direction(light_direction);
	Set_Viewport_Matrix(camera.Update_Camera(time_step));
}


void Sandbox::User_Render(Bolt::Renderer* renderer)
{
	renderer->Submit_3D_Model(uzi);
	renderer->Submit_3D_Model(skull_table);
	renderer->Submit_3D_Model(bottle_1);
	renderer->Submit_3D_Model(bottle_2);
	renderer->Submit_3D_Model(ball_1);
	renderer->Submit_3D_Model(ball_2);
} 


void Sandbox::User_App_Exit()
{
	
}



