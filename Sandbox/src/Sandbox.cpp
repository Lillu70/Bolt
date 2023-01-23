#include "Sandbox.h"
#include <iostream>
#include "Bolt/Core/Obj_Loader.h"

void Sandbox::User_Initialize(Bolt::App_Init& init_params)
{
	init_params.clear_color = glm::vec3(0.1f, 0.01f, 0.01f);

	auto names = Asset().Load_Model_File("SPEEDSTERR.obj");

	for(auto& [mesh_name, material_name] : names)
	{
		Bolt::Render_Object ro;
		ro.material = Asset().Material(material_name.c_str());
		ro.mesh = Asset().Mesh(mesh_name.c_str());
		ro.transform = glm::translate(glm::mat4(1), glm::vec3(0, 0, -1));

		m_render_objects.push_back(ro);
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

