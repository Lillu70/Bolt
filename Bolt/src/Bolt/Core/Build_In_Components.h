#pragma once

#include "../Vulkan/Vk_Resources.h"

#include "Component_System.h"
#include "Core.h"

namespace Bolt
{
	struct Transform
	{
		//http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/

		Transform() = default;

		Transform(Transform* root) 
		{ 
			if(root) Make_Child_Of(root); 
		}

		Transform(glm::vec3 local_position) 
		{ 
			Set_Local_Position(local_position); 
		}

		Transform(glm::vec3 local_position, glm::vec3 local_orientation) 
		{ 
			Set_Local_Position(local_position);
			Set_Local_Orientation(local_orientation);
		}

		Transform(glm::vec3 local_position, glm::vec3 local_orientation, glm::vec3 local_scale)
		{
			Set_Local_Position(local_position);
			Set_Local_Orientation(local_orientation);
			Set_Local_Scale(local_scale);
		}

		Transform(Transform* root, glm::vec3 local_position, glm::vec3 local_orientation, glm::vec3 local_scale)
		{
			if(root) Make_Child_Of(root);
			Set_Local_Position(local_position);
			Set_Local_Orientation(local_orientation);
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

		void Offset_Local_Position_Relative_To_Orientation(glm::vec3 offset)
		{
			m_position = m_position + (glm::quat(m_orientation) * (offset));
			
			Update_Transform_Matrix();
		}

		glm::vec3 Position() 
		{
			if (Is_Root()) return m_position;
			return  m_parent->Position() + (m_parent->Quaternion_Orientation() * ((m_position * m_parent->Scale())));
		}

		const glm::vec3& Local_Orientation() { return m_orientation; };

		void Set_Local_Orientation(glm::vec3 new_orientation)
		{
			m_orientation = new_orientation;
			Update_Transform_Matrix();
		}

		void Offset_Local_Orientation(glm::vec3 offset)
		{
			m_orientation += offset;
			Update_Transform_Matrix();
		}

		glm::vec3 Orientation()
		{
			if (!m_parent) return m_orientation;
			return m_orientation + m_parent->Orientation();
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
			m_orientation = rotation;
			m_scale = scale;
			Update_Transform_Matrix();
		}

		void Update_Transform_Matrix()
		{
			m_matrix = glm::mat4(1);
			m_matrix *= glm::translate(m_matrix, Position());

			glm::quat rotation_quaternion = Quaternion_Orientation();
			glm::mat4 rotation_matrix = glm::toMat4(rotation_quaternion);

			m_matrix *= rotation_matrix;

			m_matrix *= glm::scale(glm::mat4(1), Scale());

			for (Transform* child : m_children)
				child->Update_Transform_Matrix();
		}

		const glm::mat4* Get_Matrix() const { return &m_matrix; };

		bool Is_Root() { return m_parent == nullptr; }
		
		void Make_Root()
		{
			if (!m_parent) return;

			m_parent->Remove_Child(this);
			m_parent = nullptr;
			Update_Transform_Matrix();
		}

		void Make_Child_Of(Transform* parent) 
		{
			Internal_Make_Child_Of(parent);
			Update_Transform_Matrix();
		}

		const std::vector<Transform*> Build_Upwards_Hierarchy() const
		{
			std::vector<Transform*> hierarchy;

			Transform* p = m_parent;
			while (p)
			{
				hierarchy.push_back(p);
				p = p->m_parent;
			}

			return hierarchy;
		}

		const std::vector<Transform*> Build_Downwards_Hierarchy() const
		{
			std::vector<Transform*> hierarchy;

			for (Transform* child : m_children)
				child->Build_Downwards_Hierarchy_Internal(hierarchy);
		
			return hierarchy;
		}

		void Explode()
		{
			for (Transform* child : m_children)
				Explode_Internal(child);
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

		void Remove_Child(Transform* child)
		{
			u32 child_index = std::numeric_limits<u32>::max();
			for (child_index = 0; child_index < m_children.size(); child_index++)
				if (m_children[child_index] == child)
					break;
			
			if (child_index >= m_children.size())
				ERROR("Child not found!");

			m_children[child_index] = m_children.back();
			m_children.pop_back();
		}

		glm::quat Quaternion_Orientation()
		{
			if (Is_Root()) return glm::quat( m_orientation );
			return m_parent->Quaternion_Orientation() * glm::quat(m_orientation);
		}

		static void Explode_Internal(Transform* target)
		{
			target->Offset_Local_Position(target->m_position);
			for (Transform* child : target->m_children)
				Explode_Internal(child);
		}

		void Build_Downwards_Hierarchy_Internal(std::vector<Transform*>& hierarchy)
		{
			hierarchy.push_back(this);
			for (Transform* child : m_children)
				child->Build_Downwards_Hierarchy_Internal(hierarchy);
		}

	private:
		glm::vec3 m_position = glm::vec3(0);
		glm::vec3 m_orientation = glm::vec3(0);
		//glm::quat m_rotation = glm::quat(m_euler_rotation);
		glm::vec3 m_scale = glm::vec3(1);

		glm::mat4 m_matrix = glm::mat4(1);

		Transform* m_parent = nullptr;
		std::vector<Transform*> m_children;
	};

	struct Transform_Glue
	{
		Transform_Glue(Transform* target, Transform* self) : target(target), self(self) {}

		void Glue()
		{
			self->Set_Local_Position(target->Local_Position());
		}

		Transform* target;
		Transform* self;
	};

	struct Camera
	{
		enum class Projection
		{
			Perspective,
			Orthographic,
		};

		Camera(Entity entity) : transform(entity.Get<Transform>()) {}
		
		Transform& transform;
		glm::vec3 up_direction = glm::vec3(0, 1, 0);
		Projection projection_mode = Projection::Perspective;
		f32 field_of_view = 60.f;
		f32 far_clip_distance = 1000.f;

		void Set_Field_Of_View(f32 new_value)
		{
			new_value = std::clamp(new_value, -155.f, 155.f);
			field_of_view = new_value;
		}
	};

	struct Camera_Controller
	{
		Camera_Controller(Entity entity) : camera(entity.Get<Camera>()) { Update_Camera_Rotation(); }

		void Move(glm::vec3 direction, f32 amplitude)
		{
			if (direction == glm::vec3(0) || amplitude == 0) return;
			direction = glm::normalize(direction) * glm::vec3(amplitude);
			camera.transform.Offset_Local_Position(direction);
		}

		void Relative_Move(glm::vec3 direction, f32 amplitude)
		{
			if (direction == glm::vec3(0) || amplitude == 0) return;

			glm::vec3 horizontal_direction = glm::normalize(glm::cross(front, camera.up_direction));
			glm::vec3 vertical_direction = glm::normalize(glm::cross(front, horizontal_direction));
			glm::vec3 forward_direction = front;

			horizontal_direction = horizontal_direction * direction.x;
			vertical_direction = vertical_direction * direction.y ;
			forward_direction = forward_direction * direction.z;
			
			direction = glm::normalize(horizontal_direction + vertical_direction + forward_direction) * amplitude;

			camera.transform.Offset_Local_Position(direction);
		}

		void Rotate(f32 _yaw, f32 _pich, f32 amplitude)
		{
			f32 amp_yaw = _yaw * amplitude;
			f32 amp_pitch = _pich * amplitude;

			if (amp_yaw == 0 && amp_pitch == 0) return;

			yaw += amp_yaw;
			pich += amp_pitch;
			
			constexpr f32 max_pich = 180;
			if (pich > max_pich)
				pich = -max_pich + (pich - max_pich);

			if (pich < -max_pich)
				pich = max_pich + (pich + max_pich);
			
			if (pich < -90.f || pich > 90)
				camera.up_direction = glm::vec3(0,-1,0);
			else
				camera.up_direction = glm::vec3(0, 1, 0);

			Update_Camera_Rotation();
		}

		void Update_Camera_Rotation()
		{
			front.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pich));
			front.y = std::sin(glm::radians(pich));
			front.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pich));
			
			camera.transform.Set_Local_Orientation(front);
		}

		Camera& camera;
		f32 yaw = 90.0;
		f32 pich = 0;
		glm::vec3 front = glm::vec3(0);
	};

	struct Mesh_Renderer_Create_Info
	{
		Transform* root = nullptr;
		u32 render_pass = 1;
		Shader shader = Shader(SHADER_DEF_SPECULAR);
		glm::vec3 position = glm::vec3(0);
		glm::vec3 rotation = glm::vec3(0);
		glm::vec3 scale = glm::vec3(1);
		bool offset_by_origin = false;

#ifdef _DEBUG
		bool include_name_tag = true;
#else
		bool include_name_tag = false;
#endif // _DEBUG
	};

	struct Mesh_Renderer
	{
		Mesh_Renderer(Entity entity, Mesh* mesh, Material* material, u32 subpass_index = 1) : mesh(mesh), material(material), transform(entity.Get<Transform>()), subpass_index(subpass_index) {}

		Mesh* mesh = nullptr;
		Material* material = nullptr;
		Transform& transform;
		u32 subpass_index;
	};

	struct Billboard_Renderer
	{
		Billboard_Renderer(Entity entity, Mesh* mesh, Material* material, u32 subpass_index = 1) : mesh(mesh), material(material), transform(entity.Get<Transform>()), subpass_index(subpass_index) {}

		glm::vec3 Position() 
		{
			return transform.Position();
		}
		glm::vec2 Scale() { return transform.Scale(); }

		Mesh* mesh = nullptr;
		Material* material = nullptr;
		Transform& transform;
		u32 subpass_index;
	};

	//Unique name for the component.
	struct Name_Tag
	{
		Name_Tag(CMPT_Pools& pools, const std::string& name)
		{
			m_original_name = name;

			u32 name_matches = 0;
			for (auto name_tag : pools.Components<Name_Tag>())
				if (name_tag->m_original_name == name) name_matches++;

			if (name_matches)
				m_unique_name = m_original_name + "_" + std::to_string(name_matches);
			else 
				m_unique_name = m_original_name;
		}
		
		const std::string& Get_Unique()		{ return m_unique_name;		}
		const std::string& Get_Original()	{ return m_original_name;	}

	private:
		std::string m_original_name;
		std::string m_unique_name;
	};
}