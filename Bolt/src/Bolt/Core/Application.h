#pragma once

#include "../Vulkan/Vk_Renderer.h"

#include "App_Init_Params.h"
#include "Assets.h"
#include "Input.h"
#include "Time.h"

namespace Bolt
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	protected:
		Bolt::Input& Input();
		Assets& Asset();
		Time Time_Step();
		void Set_Global_Light_Source_Direction(glm::vec3 new_direction);
		void Set_Viewport_Matrix(glm::mat4 new_matrix);
		void Renderer_Submit(Render_Object_3D_Model& render_obj) { m_render_submissions.push_back(render_obj); }
		void Renderer_Submit(Render_Object_Billboard& render_obj) { m_render_submissions_billboard.push_back(render_obj); }

	public: //Temporary hacks.
		Vk_Renderer& _Renderer() { return *(m_renderer.get()); };

	protected:
		virtual void User_Initialize(App_Init& init_params) = 0;
		virtual void User_Update(Time time_step) = 0;
		virtual void User_App_Exit() {};

	private:
		void Apply_Init_Params(const App_Init& init_params);
		void Update_Time_Step(Time frame_time);

		std::unique_ptr<Vk_Renderer> m_renderer = nullptr;
		std::unique_ptr<Assets> m_assets = nullptr;

		std::vector<Render_Object_3D_Model> m_render_submissions;
		std::vector<Render_Object_Billboard> m_render_submissions_billboard;

	private:
		GLFWwindow* m_window = nullptr;
		std::unique_ptr<Bolt::Input> m_input = nullptr;

		glm::vec3 m_clear_color = glm::vec3(0);
		bool m_close_app = false;

		uint32_t m_fps = 0;
		uint32_t m_fps_frame_counter = 0;
		Time m_fps_accum_time = 0;
		Time m_time_step = 0;

		std::string m_window_title;
	};
}


