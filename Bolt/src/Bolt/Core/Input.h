#pragma once
#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Bolt
{
	enum class Key
	{
		Unknown = -1,
		Space = 32,
		Apostrophe = 39,
		Comma = 44,
		Minus = 45,
		Period = 46,
		Slash = 47,
		Num_0 = 48,
		Num_1 = 49,
		Num_2 = 50,
		Num_3 = 51,
		Num_4 = 52,
		Num_5 = 53,
		Num_6 = 54,
		Num_7 = 55,
		Num_8 = 56,
		Num_9 = 57,
		Semicolon = 59,
		Equals = 61,
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		Left_Bracket = 91,
		Backlash = 92,
		Right_Bracket = 93,
		Grave_Accent = 96,
		ESC = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		Page_Up = 266,
		Page_Down = 267,
		Home = 268,
		End = 269,
		Caps_Lock = 280,
		Scroll_Lock = 281,
		Num_Lock = 282,
		Print_Screen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,
		KP_0 = 320,
		KP_1 = 321,
		KP_2 = 322,
		KP_3 = 323,
		KP_4 = 324,
		KP_5 = 325,
		KP_6 = 326,
		KP_7 = 327,
		KP_8 = 328,
		KP_9 = 329,
		KP_Decimal = 330,
		KP_Devide = 331,
		KP_Multiply = 332,
		KP_Subtract = 333,
		KP_ADD = 334,
		KP_Enter = 335,
		KP_Equal = 336,
		Left_Shift = 340,
		Left_CTRL = 341,
		Left_ALT = 342,
		Left_Super = 343,
		Right_Shift = 344,
		Right_CTRL = 345,
		Right_ALT = 346,
		Right_Super = 347,
		Menu = 348,
		Mouse_Left = 0,
		Mouse_Right = 1,
		Mouse_Middle = 2,
		Mouse_4 = 3,
		Mouse_5 = 4,
		Mouse_6 = 5,
		Mouse_7 = 6,
		Mouse_8 = 7,
	};

	struct Key_State
	{
		Key_State(int key_code) : m_key_state(key_code) {}

		bool Is_Pressed() { return m_key_state == GLFW_PRESS; }
		bool Is_Released() { return m_key_state == GLFW_RELEASE; }
		bool Is_Repeated() { return m_key_state == GLFW_REPEAT; }

	private:
		const int m_key_state;
	};

	class Input final
	{
	public:
		Input(GLFWwindow* window) : m_window(window) {};

		Key_State Get(Key key)
		{
			int iKey = (int)key;

			//Mouse buttons.
			if (iKey >= 0 && iKey <= 8)
				return Key_State(glfwGetMouseButton(m_window, iKey));

			//Keyboard buttons.
			return Key_State(glfwGetKey(m_window, iKey));
		}

		glm::vec2 Mouse_Position()
		{
			double x_pos, y_pos;
			glfwGetCursorPos(m_window, &x_pos, &y_pos);

			return glm::vec2(x_pos, y_pos);
		}

	private:
		GLFWwindow* m_window;
	};
}