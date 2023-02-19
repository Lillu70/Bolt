#pragma once

#include "Key_Codes.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <array>

namespace Bolt
{
	class Application;

	class Input final
	{
		friend Application;

	public:
		Input(GLFWwindow* window);

		Key_State_Inspector Key_State(Key key);
		void Print_Key_State(Bolt::Key_State state);

		glm::vec2 Mouse_Position()
		{
			double x_pos, y_pos;
			glfwGetCursorPos(m_window, &x_pos, &y_pos);

			return glm::vec2(x_pos, y_pos);
		}

	private:
		void Clear_Key_States();
		void Update_Key_States();

	private:
		GLFWwindow* m_window;

		std::array<bool, u32(Key::COUNT)>  m_key_states[2];
		std::array<bool, u32(Key::COUNT)>* m_curr_frame = &m_key_states[1];
		std::array<bool, u32(Key::COUNT)>* m_last_frame = &m_key_states[0];
	};
}