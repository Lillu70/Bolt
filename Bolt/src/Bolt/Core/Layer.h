#pragma once

#include "ECS.h"
#include "Time.h"

namespace Bolt
{
	class Application;

	class Layer
	{
		friend Application;

	protected:
		virtual void Update(Time time_step) {};

	private:
		ECS m_ecs;
	};
}


