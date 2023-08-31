#pragma once

namespace vge
{
	class EngineLoop
	{
	public:
		static void Start();

		static void UpdateTime(const float nowTime);

	public:
		static float StartTime;
		static float LastTime;
		static float DeltaTime;
	};
}