#pragma once

#include "../Vulkan/Vk_Resources.h"

#include "Component_System.h"
#include <glm/glm.hpp>

namespace Bolt
{
	struct Transform
	{
		Transform() = default;

		Transform(Transform* root) 
		{ 
			if(root) Make_Child_Of(root); 
		}

		Transform(glm::vec3 local_position) 
		{ 
			Set_Local_Position(local_position); 
		}

		Transform(glm::vec3 local_position, glm::vec3 local_rotation) 
		{ 
			Set_Local_Position(local_position);
			Set_Local_Rotation(local_rotation);
		}

		Transform(glm::vec3 local_position, glm::vec3 local_rotation, glm::vec3 local_scale)
		{
			Set_Local_Position(local_position);
			Set_Local_Rotation(local_rotation);
			Set_Local_Scale(local_scale);
		}

		Transform(Transform* root, glm::vec3 local_position, glm::vec3 local_rotation, glm::vec3 local_scale)
		{
			if(root) Make_Child_Of(root);
			Set_Local_Position(local_position);
			Set_Local_Rotation(local_rotation);
			Set_Local_Scale(local_scale);
		}

		const glm::vec3& Local_Position() { return m_position; }

		void Set_Local_Position(glm::vec3 new_position)
		{
			m_position = new_position;
			Update_Transform_Matrix();
		}

		void Offset_Local_Position(glm::vec3 offset)
		{
			m_position += offset;
			Update_Transform_Matrix();
		}

		glm::vec3 Position() 
		{
			if (!m_parent) return m_position;
			return m_parent->Scale() * m_position + m_parent->Position();
		}

		const glm::vec3& Local_Rotation() { return m_rotation; };

		void Set_Local_Rotation(glm::vec3 new_rotation)
		{
			m_rotation = new_rotation;
			Update_Transform_Matrix();
		}

		void Offset_Local_Rotation(glm::vec3 offset)
		{
			m_rotation += offset;
			Update_Transform_Matrix();
		}

		glm::vec3 Rotation()
		{
			if (!m_parent) return m_rotation;
			return m_rotation + m_parent->Rotation();
		}

		const glm::vec3& Local_Scale() { return m_scale; }

		void Set_Local_Scale(glm::vec3 new_scale)
		{
			m_scale = new_scale;
			Update_Transform_Matrix();
		}

		void Offset_Local_Scale(glm::vec3 offset)
		{
			m_scale += offset;
			Update_Transform_Matrix();
		}

		glm::vec3 Scale()
		{
			if (!m_parent) return m_scale;
			return m_scale * m_parent->Scale();
		}

		void Set_Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, Transform* root = nullptr)
		{
			if (root) Internal_Make_Child_Of(root);

			m_position = position;
			m_rotation = rotation;
			m_scale = scale;
			Update_Transform_Matrix();
		}

		void Update_Transform_Matrix()
		{
			m_matrix = glm::mat4(1);
			m_matrix *= glm::translate(m_matrix, Position());
			m_matrix *= glm::scale(glm::mat4(1), Scale());

			for (Transform* child : m_children)
				child->Update_Transform_Matrix();
		}

		glm::mat4* Get_Matrix() { return &m_matrix; };

		void Make_Root()
		{

		}

		void Make_Child_Of(Transform* parent) 
		{
			Internal_Make_Child_Of(parent);
			Update_Transform_Matrix();
		}

	private:
		inline void Internal_Make_Child_Of(Transform* parent)
		{
			ASSERT(parent, "Parent is a nullptr!");
			ASSERT(parent != this, "Can't parent to self!");

			if (m_parent != nullptr)
				Make_Root();

			m_parent = parent;
			m_parent->m_children.push_back(this);
		}

	private:
		glm::vec3 m_position = glm::vec3(0);
		glm::vec3 m_rotation = glm::vec3(0);
		glm::vec3 m_scale = glm::vec3(1);

		glm::mat4 m_matrix = glm::mat4(1);

		Transform* m_parent = nullptr;
		std::vector<Transform*> m_children;
	};
	
	struct Mesh_Renderer
	{
		Mesh_Renderer(Entity entity, Mesh* mesh, Material* material) : mesh(mesh), material(material), transform(entity.Get<Transform>()) {}

		Mesh* mesh = nullptr;
		Material* material = nullptr;
		Transform& transform;
	};

	struct Billboard_Renderer
	{
		Billboard_Renderer(Entity entity, Mesh* mesh, Material* material) : mesh(mesh), material(material), transform(entity.Get<Transform>()) {}

		glm::vec3 Position() 
		{
			return transform.Position();
		}
		glm::vec2 Scale() { return transform.Scale(); }

		Mesh* mesh = nullptr;
		Material* material = nullptr;
		Transform& transform;
	};
}