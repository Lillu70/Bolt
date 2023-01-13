#pragma once

#include <chrono>

#include "Time.h"

namespace Bolt
{
	class Timer final
	{
	public:
		Timer() : m_start_time(std::chrono::high_resolution_clock::now()) {}
		Timer(Timer& other) : m_start_time(other.m_start_time) {}

		void Start() { *this = Timer(); }
		Time Get_Time() const
		{
			return Time(std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - m_start_time).count());
		}

	private:
		std::chrono::steady_clock::time_point m_start_time;
	};
}