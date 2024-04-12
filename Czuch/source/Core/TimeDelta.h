#pragma once

namespace Czuch
{
	struct TimeDelta
	{
	public:
		TimeDelta(float delta): m_DeltaTime(delta) {}
		~TimeDelta()=default;
		const float GetDeltaTime() const { return m_DeltaTime; }
		const float GetDeltaTimeInMilliseconds() const { return m_DeltaTime * 1000.0f; }
	private:
		float m_DeltaTime;
	};
}



