#pragma once

#include "Common.h"
#include "ECS/Entity.h"
#include "ECS/Coordinator.h"

namespace vge
{
	struct RenderComponent
	{
	public:
		RenderComponent() = default;
		RenderComponent(i32 modelId) : ModelId(modelId) {}

	public:
		i32 ModelId = INDEX_NONE;
	};
}
