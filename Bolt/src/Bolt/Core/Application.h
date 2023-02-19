#pragma once

#include "../Vulkan/Vk_Renderer.h"

#include "App_Init_Params.h"
#include "Assets.h"
#include "Input.h"
#include "Layer.h"
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

		void Push_Layer(std::unique_ptr<Layer> layer);

	protected:
		virtual void User_Initialize(App_Init& init_params) = 0;
		
		virtual void User_Update(Time time_step) = 0;
		
		virtual void User_App_Exit() {};

	private:
		void Apply_Init_Params(const App_Init& init_params);
		
		void Update_Time_Step(Time frame_time);

		std::unique_ptr<Vk_Renderer> m_renderer = nullptr;
		std::unique_ptr<Assets> m_assets = nullptr;

	private:
		GLFWwindow* m_window = nullptr;
		std::string m_window_title;
		std::unique_ptr<Bolt::Input> m_input = nullptr;

		glm::vec3 m_clear_color = glm::vec3(0);
		bool m_close_app = false;

		u32 m_accum_draw_call_count = 0;
		u32 m_fps = 0;
		u32 m_fps_frame_counter = 0;
		Time m_fps_accum_time = 0;
		Time m_time_step = 0;

		std::vector<std::unique_ptr<Layer>> m_layers;
		Render_Submissions m_submissions;
	};
}


