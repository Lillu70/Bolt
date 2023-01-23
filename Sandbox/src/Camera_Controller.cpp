#include "Camera_Controller.h"



glm::mat4 Camera_Controller::Update_Camera(float time_step)
{
	Cursor_Camera_Rotation(time_step);
	Arrow_Key_Camera_Rotation(time_step);

	glm::vec3 up_axis = glm::vec3(0.f, 1.f, 0.f);
	const float max_pich = 180;
	if (m_pich > max_pich)
		m_pich = -max_pich + (m_pich - max_pich);

	if (m_pich < -max_pich)
		m_pich = max_pich + (m_pich + max_pich);

	if (m_pich < -90.f || m_pich > 90)
		up_axis *= -1.f;

	glm::vec3 front;
	front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pich));
	front.y = std::sin(glm::radians(m_pich));
	front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pich));
	m_camera_front = glm::normalize(front);

	float move_speed = m_camera_movement_speed * time_step;
	if (m_input.Get(Bolt::Key::W).Is_Pressed())
		if (m_input.Get(Bolt::Key::Left_Shift).Is_Pressed())
			m_camera_position += move_speed * m_camera_front;
		else
			m_camera_position -= move_speed * glm::normalize(glm::cross(m_camera_front, glm::normalize(glm::cross(m_camera_front, up_axis))));

	if (m_input.Get(Bolt::Key::S).Is_Pressed())
		if (m_input.Get(Bolt::Key::Left_Shift).Is_Pressed())
			m_camera_position -= move_speed * m_camera_front;
		else
			m_camera_position += move_speed * glm::normalize(glm::cross(m_camera_front, glm::normalize(glm::cross(m_camera_front, up_axis))));

	if (m_input.Get(Bolt::Key::A).Is_Pressed())
		m_camera_position -= move_speed * glm::normalize(glm::cross(m_camera_front, up_axis));

	if (m_input.Get(Bolt::Key::D).Is_Pressed())
		m_camera_position += move_speed * glm::normalize(glm::cross(m_camera_front, up_axis));

	return glm::lookAt(m_camera_position, m_camera_position + m_camera_front, up_axis);
}

void Camera_Controller::Arrow_Key_Camera_Rotation(float time_step)
{
	float rotation_speed = m_camera_key_rotation_speed * time_step;
	if (m_input.Get(Bolt::Key::Right).Is_Pressed())
		m_yaw += rotation_speed;
	if (m_input.Get(Bolt::Key::Left).Is_Pressed())
		m_yaw -= rotation_speed;
	if (m_input.Get(Bolt::Key::Up).Is_Pressed())
		m_pich += rotation_speed;
	if (m_input.Get(Bolt::Key::Down).Is_Pressed())
		m_pich -= rotation_speed;
}

void Camera_Controller::Cursor_Camera_Rotation(float time_step)
{
	glm::vec2 mouse_position = m_input.Mouse_Position();
	glm::vec2 mouse_offset = mouse_position - m_last_mouse_position;
	m_last_mouse_position = mouse_position;

	if (m_input.Get(Bolt::Key::Mouse_Left).Is_Released()) return;

	glm::vec2 camera_movement = mouse_offset * m_camera_cursor_rotation_speed;
	m_yaw += camera_movement.x;
	m_pich += -camera_movement.y;
}
