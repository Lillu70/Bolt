#pragma once
#include "Vk_Resources.h"

namespace Bolt
{
	class Renderer 
	{
	public:
		virtual void Submit_3D_Model(Mesh* mesh, Material* material, glm::mat4 transform) = 0;
		virtual void Submit_3D_Model(Render_Object& render_obj) = 0;
	};
}