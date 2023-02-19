#include "Input.h"

Bolt::Input::Input(GLFWwindow* window) : m_window(window)
{
	glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
	Clear_Key_States();
}

Bolt::Key_State_Inspector Bolt::Input::Key_State(Key key)
{
	u32 i = u32(key);
	bool curr_state = (*m_curr_frame)[i];
	bool last_state = (*m_last_frame)[i];

	if (curr_state	&&  last_state)	return Key_State_Inspector(Bolt::Key_State::Down);
	if (curr_state	&& !last_state)	return Key_State_Inspector(Bolt::Key_State::Pressed);
	if (!curr_state && !last_state) return Key_State_Inspector(Bolt::Key_State::Up);
	if (!curr_state &&  last_state) return Key_State_Inspector(Bolt::Key_State::Released);
	
	return Key_State_Inspector(Key_State::Down);
}

void Bolt::Input::Print_Key_State(Bolt::Key_State state)
{
	switch (state)
	{
	case Bolt::Key_State::Pressed:
		std::cout << "PRESSED\n";
		break;
	case Bolt::Key_State::Released:
		std::cout << "RELEASED\n";
		break;
	case Bolt::Key_State::Up:
		std::cout << "UP\n";
		break;
	case Bolt::Key_State::Down:
		std::cout << "DOWN\n";
		break;
	}
}

void Bolt::Input::Clear_Key_States()
{
	for (u32 i = 0; i < 2; i++)
		for (u32 y = 0; y < m_key_states[i].size(); y++)
			m_key_states[i][y] = false;
}

void Bolt::Input::Update_Key_States()
{
	std::swap(m_curr_frame, m_last_frame);
	
	//Get the state of the keyboard keys.
	for (u32 i = 0; i <= u32(Key::MENU); i++)
		(*m_curr_frame)[i] = glfwGetKey(m_window, Convert_Key_Index_To_GLFW_Value(i));

	//Get the state of the mouse buttons.
	for (u32 i = u32(Key::MENU) + 1; i < u32(Key::COUNT); i++)
		(*m_curr_frame)[i] = glfwGetMouseButton(m_window, Convert_Key_Index_To_GLFW_Value(i));
}
