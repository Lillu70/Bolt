#pragma once
#include "../Vulkan/Renderer_Resource_Factory.h"

namespace Bolt
{
	class Assets_Resource_Injector
	{
	public:
		virtual void Push_Mesh(const std::string& name, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) = 0;
		virtual void Push_Material(const std::string& model_name, const std::string material_name, Material_Data& material_data) = 0;
	};
}

