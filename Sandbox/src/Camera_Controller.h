#pragma once

#include <Bolt.h>

class Camera_Controller final
{
public:
	Camera_Controller(Bolt::Input& input) : m_input(input) {};

	glm::mat4 Update_Camera(float time_step);

private:
	void Arrow_Key_Camera_Rotation(float time_step);
	void Cursor_Camera_Rotation(float time_step);

public:
	float m_camera_movement_speed = 10;
	float m_camera_key_rotation_speed = 100;
	float m_camera_cursor_rotation_speed = 0.5;

private:
	Bolt::Input& m_input;

	glm::vec3 m_camera_position = glm::vec3(0.f);
	glm::vec3 m_camera_front = glm::vec3(0.f, 0.f, -1.f);

	float m_yaw = -90.0f;
	float m_pich = 0;

	glm::vec2 m_last_mouse_position = glm::vec2(0);
};

