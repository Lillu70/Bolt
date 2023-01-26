#include "Sandbox.h"
#include <iostream>
#include "Bolt/Core/Obj_Loader.h"

void Sandbox::User_Initialize(Bolt::App_Init& init_params)
{
	init_params.clear_color = glm::vec3(0.1f, 0.01f, 0.01f);
	
	Spawn_Model_3D();
}


void Sandbox::User_Update(Bolt::Time time_step)
{
	m_light_direction = glm::rotate(m_light_direction, m_light_rotation_speed * time_step, glm::vec3(0, 1, 0));

	Set_Global_Light_Source_Direction(m_light_direction);
	Set_Viewport_Matrix(m_camera.Update_Camera(time_step));

	if (Input().Get(Bolt::Key::O).Is_Pressed() && spawn_timer.Get_Time().As_Seconds() > 1.f)
	{
		for(uint32_t i = 0; i < 10; i++)
			Spawn_Billboard();
		
		static int boo = 1;

		last_pos = glm::vec3(0,0, -5 - boo++ * -5.f);
		spawn_timer.Start();
	}
}


void Sandbox::Spawn_Billboard()
{
	int i = 0;
	glm::vec3 off = glm::vec3(1);
	for (auto& [mesh_name, material_name] : Asset().Load_Model_File("point_light.obj", Bolt::Shader(SHADER_DEF_BILLBOARD)))
	{
		Bolt::Render_Object_Billboard ro;
		ro.material = Asset().Material(material_name);
		ro.mesh = Asset().Mesh(mesh_name);
		ro.position = last_pos;

		Renderer_Submit(ro);
	}

	last_pos += offset;
}

void Sandbox::Spawn_Model_3D()
{
	int i = 0;
	glm::vec3 off = glm::vec3(1);
	for (auto& [mesh_name, material_name] : Asset().Load_Model_File("chainsaw.obj"))
	{
		Bolt::Render_Object_3D_Model ro;
		ro.material = Asset().Material(material_name);
		ro.mesh = Asset().Mesh(mesh_name);
		ro.transform = glm::translate(glm::mat4(1), last_pos /*+ glm::vec3(1.f * i++, 0,0)*/);

		Renderer_Submit(ro);
	}

	last_pos += offset;
}

