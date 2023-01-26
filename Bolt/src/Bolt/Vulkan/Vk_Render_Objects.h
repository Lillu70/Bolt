#pragma once
#include "Vk_Resources.h"

namespace Bolt
{
	struct Render_Object_3D_Model
	{
		Render_Object_3D_Model() = default;
		Render_Object_3D_Model(Mesh* mesh, Material* material, glm::mat4 transform) : mesh(mesh), material(material), transform(transform) {}

		Mesh* mesh = nullptr;
		Material* material = nullptr;
		glm::mat4 transform = glm::mat4(0);
	};

	struct Render_Object_Billboard
	{
		Render_Object_Billboard() = default;
		Render_Object_Billboard(Mesh* mesh, Material* material, glm::vec3 position, glm::vec2 scale) : mesh(mesh), material(material), position(position), scale(scale) {}

		Mesh* mesh = nullptr;
		Material* material = nullptr;

		glm::vec3 position;
		glm::vec2 scale;
	};

	struct Render_Object_Cicle
	{
		glm::vec3 position;
		float radius;
		float thickness;
	};

	struct Render_Object_Line
	{
		glm::vec3 position_start;
		glm::vec3 position_end;
		float thickness;
	};

	struct Renderer_Submissions
	{
		std::vector<const std::vector<Render_Object_3D_Model>*> m_models_3D;
		std::vector<const std::vector<Render_Object_Billboard>*> m_billboards;

		void Clear()
		{
			m_models_3D.clear();
			m_billboards.clear();
		}
	};

	struct Scoped_Clear_Renderer_Submissions
	{
		Scoped_Clear_Renderer_Submissions(Renderer_Submissions& target) : target(target) {}
		
		~Scoped_Clear_Renderer_Submissions()
		{
			target.Clear();
		}

	private:
		Renderer_Submissions& target;
	};
}

