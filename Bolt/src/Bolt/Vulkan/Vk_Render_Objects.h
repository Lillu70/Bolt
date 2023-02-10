#pragma once
#include "Vk_Resources.h"

namespace Bolt
{
	struct Render_Object_3D_Model
	{
		Render_Object_3D_Model() = default;
		Render_Object_3D_Model(Mesh* mesh, Material* material, const glm::mat4* transform) : mesh(mesh), material(material), transform(*transform) {}

		Mesh* mesh = nullptr;
		Material* material = nullptr;
		glm::mat4 transform;
	};

	struct Render_Object_Billboard
	{
		Render_Object_Billboard() = default;
		Render_Object_Billboard(Mesh* mesh, Material* material, glm::vec3 position, glm::vec2 scale) : mesh(mesh), material(material), position(position), scale(scale) {}

		Mesh* mesh = nullptr;
		Material* material = nullptr;

		glm::vec3 position = glm::vec3(0);
		glm::vec2 scale = glm::vec2(1);
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

	struct Render_Pass_Info
	{
		Render_Pass* render_pass = nullptr;
		Scene_Descriptor* scene_descriptor = nullptr;
		u32 data_id = 0;
	};
}