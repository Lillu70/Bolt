#pragma once
#include "Vk_Resources.h"

namespace Bolt
{
	struct Render_Object_3D_Model
	{
		Render_Object_3D_Model() = default;
		Render_Object_3D_Model(Mesh* mesh, Material* material, const glm::mat4* transform) : mesh(mesh), material(material), transform(*transform) {}
		Render_Object_3D_Model(Mesh* mesh, Material* material, const glm::mat4* transform, f32 sqrd_distance_to_camera) : 
			mesh(mesh), material(material), transform(*transform), sqrd_distance_to_camera(sqrd_distance_to_camera) {}

		Mesh* mesh = nullptr;
		Material* material = nullptr;
		glm::mat4 transform;

		f32 sqrd_distance_to_camera = 0;
	};

	struct Render_Object_Billboard
	{
		Render_Object_Billboard() = default;
		Render_Object_Billboard(Mesh* mesh, Material* material, glm::vec3 position, glm::vec2 scale) : mesh(mesh), material(material), position(position), scale(scale) {}
		Render_Object_Billboard(Mesh* mesh, Material* material, glm::vec3 position, glm::vec2 scale, f32 sqrd_distance_to_camera) : 
			mesh(mesh), material(material), position(position), scale(scale), sqrd_distance_to_camera(sqrd_distance_to_camera){}

		Mesh* mesh = nullptr;
		Material* material = nullptr;
		glm::vec3 position = glm::vec3(0);
		glm::vec2 scale = glm::vec2(1);
		
		f32 sqrd_distance_to_camera = 0;
	};

	struct Render_Object_Cicle
	{
		glm::vec3 position = glm::vec3(0);
		glm::vec4 color = glm::vec4(1);
		f32 radius = 0.5f;
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