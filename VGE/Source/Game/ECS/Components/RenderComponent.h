#pragma once

#include "Common.h"
#include "ECS/Entity.h"
#include "ECS/Coordinator.h"

namespace vge
{
	struct RenderComponent
	{
		RenderComponent() = default;
		RenderComponent(i32 modelId) : ModelId(modelId) {}

		i32 ModelId = INDEX_NONE;
	};
}
