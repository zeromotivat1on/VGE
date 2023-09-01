#pragma once

namespace vge
{
	class RenderLoop
	{
	public:
		RenderLoop() = default;

		void Initialize();
		void Tick(float deltaTime);
		void Destroy();
	};
}
