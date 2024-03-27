#pragma once

#include "ECS/System.h"

namespace vge
{
class CameraSystem : public ecs::System
{
public:
	CameraSystem() = default;

	void Initialize();
	void Tick(f32 deltaTime);
};
}
