#pragma once

namespace Bolt
{
	struct Time
	{
		Time() = default;
		Time(float time_as_seconds) : m_time_as_seconds(time_as_seconds) {}

		float As_Seconds() const		{ return m_time_as_seconds;}
		float As_Milliseconds() const	{ return m_time_as_seconds * 0.001f; }
		float As_Microseconds() const	{ return m_time_as_seconds * 0.000001f; }
		float As_Minutes() const		{ return 60.f / m_time_as_seconds; }
		float As_Hours() const			{ return 60.f / As_Minutes(); }

		operator float() const			{ return m_time_as_seconds; }

		Time& operator = (const float _float) 
		{ 
			m_time_as_seconds = _float; 
			return *this; 
		}

		Time& operator ++ ()
		{
			m_time_as_seconds += 1.f;
			return *this;
		}

		Time& operator -- ()
		{
			m_time_as_seconds -= 1.f;
			return *this;
		}

		Time operator ++ (int)
		{
			Time old_val = m_time_as_seconds;
			operator++();
			return old_val;
		}

		Time operator -- (int)
		{
			Time old_val = m_time_as_seconds;
			operator--();
			return old_val;
		}

		Time& operator += (const Time& other)
		{
			m_time_as_seconds += other.m_time_as_seconds;
			return *this;
		}

		Time& operator -= (const Time& other)
		{
			m_time_as_seconds -= other.m_time_as_seconds;
			return *this;
		}

		Time& operator *= (const Time& other)
		{
			m_time_as_seconds *= other.m_time_as_seconds;
			return *this;
		}

		Time& operator /= (const Time& other)
		{
			m_time_as_seconds /= other.m_time_as_seconds;
			return *this;
		}

	private:
		float m_time_as_seconds = 0.f;
	};
}