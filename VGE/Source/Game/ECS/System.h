#pragma once

#include "Common.h"
#include "ECS/Entity.h"

namespace vge
{
	class System
	{
	public:
		inline void Add(Entity entity) { m_Entities.push_back(entity); }

		inline Entity* Find(Entity entity)
		{
			return algo::Find(m_Entities, entity);
		}

		inline bool Remove(Entity entity)
		{
			if (Entity* toRemove = Find(entity))
			{
				const auto pos = std::cbegin(m_Entities) + std::distance(m_Entities.data(), toRemove);
				m_Entities.erase(pos);
				return true;
			}

			return false;
		}

	protected:
		std::vector<Entity> m_Entities;
	};
}
