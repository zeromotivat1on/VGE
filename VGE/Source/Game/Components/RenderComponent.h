#pragma once

#include "Common.h"
#include "ECS/Entity.h"
#include "ECS/Coordinator.h"

namespace vge
{
	struct RenderComponent
	{
	public:
		inline static RenderComponent* GetFrom(const Entity& entity) { return GCoordinator->GetComponent<RenderComponent>(entity); }

	public:
		int32 ModelId = INDEX_NONE;
	};
}
