#pragma once

#include "../Vulkan/Render_Submissions.h"

#include "Build_In_Components.h"
#include "Assets.h"
#include "Input.h"
#include "Time.h"

namespace Bolt
{
	class Application;

	class Layer
	{
		friend Application;

	public:
		Layer() : m_entity_spawner(m_components) {}
		virtual ~Layer() {};

		virtual void Initialize() {};
		virtual void Update(Time time_step) {};

		void Inject_Render_Submissions();
		Entity_ID Create_Camera_And_Env_Data_Entity();

		bool m_visualize_transforms = false;

	protected:
		void Create_Skybox_From_Model(const std::string& model_path, glm::vec3 scale);
		void Create_Mesh_Renderers_From_Model(const std::string& model_path, Mesh_Renderer_Create_Info create_info);
		void Spawn_Entities_From_Model(const std::string& model_path, Transform* root = nullptr, glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1), u32 render_pass = 1);
		void Spawn_Billboards_From_Model(const std::string& model_path, Transform* root = nullptr, glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1), u32 render_pass = 1);
		void Spawn_Circles_From_Model(const std::string& model_path, Transform* root = nullptr, glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1), u32 render_pass = 2);


	protected:
		Entity Create_Entity();
		Entity Create_Entity(Entity_ID id);
		Entity Get_Camera_Entity(u32 index = 0);
		void Create_Entity(Entity& entity);
		glm::uvec2 Window_Extent();
		void Close_App() { *m_close_app_ptr = true; }
		
	protected:
		Assets& Asset() { return *m_assets; }
		Bolt::Input& Input() { return *m_input; }
		CMPT_Pools& ECS() { return m_components; }

	protected:
		void Push_Rendering_Subpass(Render_Pass_Info pass_info);
		u32 Layer_Index() { return m_layer_index; }
		u32 Subpass_Count() { return m_render_submissions->Active_Main_Pass_Subpass_Count(); }
	
	private:
		void Update_Scene_Descriptor(Bolt::Render_Submissions& submissions, glm::vec3& out_camera_position);

	private:
		Entity_Dispatcher m_entity_spawner;
		CMPT_Pools m_components;
		Assets* m_assets;
		Bolt::Input* m_input;
		Render_Submissions* m_render_submissions;
		u32 m_layer_index = 0;
		VkExtent2D* m_extent_ptr;
		bool* m_close_app_ptr;
	};
}



