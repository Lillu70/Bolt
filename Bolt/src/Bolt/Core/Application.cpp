#include "Application.h"

#include "Timer.h"
#include "Core.h"

namespace Bolt
{
	Application::Application()
	{
		glfwInit();

		m_window = Create_GLFW_Window(glm::uvec2(800), "Bolt - Application");
		m_input = std::make_unique<Bolt::Input>(m_window);
		m_renderer = std::make_unique<Vk_Renderer>(m_window);
		m_assets = std::make_unique<Assets>(m_renderer.get());
	}

	Application::~Application()
	{
		glfwTerminate();
	}

	void Application::Run()
	{
		App_Init init_params;
		User_Initialize(init_params);
		Apply_Init_Params(init_params);

		while (!glfwWindowShouldClose(m_window) && !m_close_app)
		{
			Timer frame_timer;

			glfwPollEvents();
			
			Input().Update_Key_States();

			User_Update(m_time_step);

			m_submissions.Activate_First_Pass();
			
			for (std::unique_ptr<Layer>& layer : m_layers)
			{
				layer->Update(m_time_step);
				layer->Inject_Render_Submissions();
				m_submissions.Activate_Next_Main_Pass();
			}
			
			m_renderer->Draw_Frame(m_submissions, m_clear_color);
			m_accum_draw_call_count += m_renderer->Last_Frame_Draw_Call_Count();
			m_submissions.Clear();

			Update_Time_Step(frame_timer.Get_Time().As_Seconds());
		}

		User_App_Exit();
	}
	
	void Application::Apply_Init_Params(const App_Init& init_params)
	{
		m_clear_color = init_params.clear_color;
		glfwSetWindowSize(m_window, (int32_t)init_params.window_dimensions.x, (int32_t)init_params.window_dimensions.y);
		
		m_window_title = init_params.window_title;
		glfwSetWindowTitle(m_window, init_params.window_title.c_str());
	}

	void Application::Update_Time_Step(Time frame_time)
	{
		m_time_step = frame_time;
		m_fps_frame_counter++;
		m_fps_accum_time += m_time_step;
		
		if (m_fps_accum_time >= 1.f)
		{
			m_fps_accum_time--;
			m_fps = m_fps_frame_counter;

			u32 mean_draw_call_count = m_accum_draw_call_count / m_fps_frame_counter;
			m_accum_draw_call_count = 0;
			m_fps_frame_counter = 0;

			std::string title = m_window_title + " [FPS: " + std::to_string(m_fps) + " | Draws: " + std::to_string(mean_draw_call_count) + "]";
			glfwSetWindowTitle(m_window, title.c_str());
		}
	}

	Input& Application::Input()
	{
		return *(m_input.get());
	}

	Assets& Application::Asset()
	{
		return *(m_assets.get());
	}

	Time Application::Time_Step()
	{
		return m_time_step;
	}

	void Application::Push_Layer(std::unique_ptr<Layer> layer)
	{
		//TODO: pass an application interface for the layer instead of this injection shit.

		ASSERT(layer.get(), "Attempting to push a nullptr!");
		auto& _layer = m_layers.emplace_back(std::move(layer));
		_layer->m_assets = m_assets.get();
		_layer->m_input = m_input.get();
		_layer->m_render_submissions = &m_submissions;
		_layer->m_layer_index = u32(m_layers.size()) - 1;
		_layer->m_extent_ptr = m_renderer->Swapchain_Extent();
		_layer->m_close_app_ptr = &m_close_app;
		
		Render_Pass_Info pass_info;
		Clear_Method color_clear_method = _layer->m_layer_index == 0? Clear_Method::Clear: Clear_Method::Load;
		pass_info.render_pass = Asset().Create_Render_Pass(m_submissions.Main_Pass_Count(), 0, color_clear_method, Clear_Method::Clear);
		pass_info.scene_descriptor = Asset().Create_Scene_Descriptor(m_submissions.Main_Pass_Count(), 0);
		pass_info.data_id = _layer->Create_Camera_And_Env_Data_Entity();
		
		m_submissions.Push_New_Main_Pass(pass_info);
		m_submissions.Activate_Last_Main_Pass();
		_layer->Initialize();
	}
}