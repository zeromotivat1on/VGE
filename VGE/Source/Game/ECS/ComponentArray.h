#pragma once

#include "Common.h"
#include "Entity.h"

namespace vge
{
	class IComponentArray
	{
	public:
		virtual ~IComponentArray() = default;
		virtual void EntityDestroyed(Entity entity) = 0;
	};

	template<typename T>
	class ComponentArray : public IComponentArray
	{
	public:
		inline void Add(Entity entity, T component)
		{
			ASSERT(m_EntityToIndex.find(entity) == m_EntityToIndex.end());

			size_t newIndex = m_Size;
			m_EntityToIndex[entity] = newIndex;
			m_IndexToEntity[newIndex] = entity;
			m_Components[newIndex] = component;

			++m_Size;
		}

		inline void Remove(Entity entity)
		{
			ASSERT(m_EntityToIndex.find(entity) != m_EntityToIndex.end());

			// Copy element at end into deleted element's place to maintain density
			size_t indexOfRemovedEntity = m_EntityToIndex[entity];
			size_t indexOfLastElement = m_Size - 1;
			m_Components[indexOfRemovedEntity] = m_Components[indexOfLastElement];

			// Update map to point to moved spot
			Entity entityOfLastElement = m_IndexToEntity[indexOfLastElement];
			m_EntityToIndex[entityOfLastElement] = indexOfRemovedEntity;
			m_IndexToEntity[indexOfRemovedEntity] = entityOfLastElement;

			m_EntityToIndex.erase(entity);
			m_IndexToEntity.erase(indexOfLastElement);

			--m_Size;
		}

		inline T& Get(Entity entity)
		{
			ASSERT(m_EntityToIndex.find(entity) != m_EntityToIndex.end());

			// Return a reference to the entity's component
			return m_Components[m_EntityToIndex[entity]];
		}

		inline void EntityDestroyed(Entity entity) override
		{
			if (m_EntityToIndex.find(entity) != m_EntityToIndex.end())
			{
				Remove(entity);
			}
		}

	private:
		size_t m_Size = 0;
		std::array<T, GMaxEntities> m_Components = {};
		std::unordered_map<Entity, size_t> m_EntityToIndex = {};
		std::unordered_map<size_t, Entity> m_IndexToEntity = {};
	};
}
