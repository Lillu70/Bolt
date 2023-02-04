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

			User_Update(m_time_step);

			for (std::unique_ptr<Layer>& layer : m_layers)
			{
				layer->Update(m_time_step);
				m_renderer->Submit(layer->Get_Render_Submissions());
			}

			m_renderer->Draw_Frame(m_clear_color);

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
			m_fps_frame_counter = 0;

			std::string title = m_window_title + " -- FPS: " + std::to_string(m_fps);
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


	void Application::Set_Global_Light_Source_Direction(glm::vec3 new_direction)
	{
		m_renderer->Set_Global_Light_Source_Direction(new_direction);
	}


	void Application::Set_Viewport_Matrix(glm::mat4 new_matrix)
	{
		m_renderer->Set_Viewport_Matrix(new_matrix);
	}

	void Application::Push_Layer(std::unique_ptr<Layer> layer)
	{
		ASSERT(layer.get(), "Attempting to push a nullptr!");
		auto& _layer = m_layers.emplace_back(std::move(layer));
		_layer->m_assets = m_assets.get();
		_layer->m_input = m_input.get();
		_layer->Initialize();
	}
}



