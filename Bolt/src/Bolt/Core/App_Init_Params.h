#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <string>

namespace Bolt
{
	struct App_Init
	{
		std::string window_title = "Bolt - Application";
		glm::uvec2 window_dimensions = glm::uvec2(800);
		glm::vec3 clear_color = glm::vec3(0);
	};
}

