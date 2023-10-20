#pragma once

#include "Common.h"
#include "ECS/Entity.h"

namespace vge
{
	class System
	{
	public:
		inline auto& GetAll() { return m_Entities; }

		inline void Add(Entity entity) { m_Entities.insert(entity); }
		inline void Append(const std::set<Entity>& entities) { for (const auto& entity : entities) { Add(entity); } }

		inline const Entity* Find(Entity entity) { return algo::Find(m_Entities, entity); }
		inline void Remove(Entity entity) { m_Entities.erase(entity); }

	protected:
		template<typename Functor>
		inline void ForEachEntity(Functor functor) const
		{
			for (const auto& entity : m_Entities)
			{
				functor(entity);
			}
		}

	protected:
		std::unordered_set<Entity> m_Entities;
	};
}
