#pragma once

namespace vge
{
	class GameLoop
	{
	public:
		GameLoop() = default;

		void Initialize();
		void Tick(float deltaTime);
		void Destroy();
	};
}
