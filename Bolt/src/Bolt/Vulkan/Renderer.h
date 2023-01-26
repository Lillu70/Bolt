#pragma once
#include "Vk_Resources.h"

namespace Bolt
{
	class Renderer 
	{
	public:
		virtual void Submit(const std::vector<Render_Object_3D_Model>& render_objects) = 0;
		virtual void Submit(const std::vector<Render_Object_Billboard>& render_objects) = 0;
	};
}