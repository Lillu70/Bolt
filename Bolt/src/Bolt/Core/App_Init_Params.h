#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>


namespace Bolt
{
	struct App_Init
	{
		glm::uvec2 window_dimensions = glm::uvec2(800);
		glm::vec3 clear_color = glm::vec3(0);
	};
}

