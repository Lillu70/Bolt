#pragma once

#include "../Vulkan/Vk_Render_Objects.h"

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
		virtual Render_Submission& Get_Render_Submissions();

	protected:
		Entity Create_Entity();
		void Create_Entity(Entity& entity);
		
		void Spawn_Entities_From_Model(const std::string& model_path, Transform* root = nullptr, glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1));
		void Spawn_Billboards_From_Model(const std::string& model_path, Transform* root = nullptr, glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1));

		Assets& Asset() { return *m_assets; }
		Bolt::Input& Input() { return *m_input; }

	private:
		Entity_Dispatcher m_entity_spawner;
		CMPT_Pools m_components;
		Assets* m_assets;
		Bolt::Input* m_input;

		Render_Submission m_submissions;
	};
}



