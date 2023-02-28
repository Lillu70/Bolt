#pragma once

#include "Core.h"
#include "../Vulkan/Vk_Internal_Components.h"

namespace Bolt
{
	class Maths
	{
	public:
		Maths() = delete;

		static f32 Distance_Squered(glm::vec3 a, glm::vec3 b)
		{
			glm::vec3 dif = a - b;
			return glm::dot(dif, dif);
		}

		static glm::vec3 Calculate_Mesh_Origin(std::vector<Vertex>& verts)
		{
			ASSERT(!verts.empty(), "Verts is empty!");

			glm::vec3 sum = glm::vec3(0);
			for (auto& vert : verts)
				sum += vert.position;

			sum /= verts.size();

			return sum;
		}
	};

	class Util
	{
	public:
		Util() = delete;

		template<typename T>
		static void  Vector_Find_Swap_Top_An_Pop(std::vector<T>& vec, T element)
		{
			auto iter = std::find(vec.begin(), vec.end(), element);
			Bolt::u64 idx = iter - vec.begin();
			vec[idx] = vec.back();
			vec.pop_back();
		}
	};
	
}