#pragma once

#include "Common.h"
#include "Entity.h"
#include "Core/Error.h"

namespace vge::ecs
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
	inline size_t GetSize() const { return m_Size; }
	inline T* GetComponents() { return _Components.data(); }

	inline void Add(Entity entity, const T& component)
	{
		ASSERT(_EntityToIndex.find(entity) == _EntityToIndex.end());

		const size_t newIndex = m_Size;
		_EntityToIndex[entity] = newIndex;
		_IndexToEntity[newIndex] = entity;
		_Components[newIndex] = component;

		++m_Size;
	}

	inline void Add(Entity entity, T&& component)
	{
		ASSERT(_EntityToIndex.find(entity) == _EntityToIndex.end());

		const size_t newIndex = m_Size;
		_EntityToIndex[entity] = newIndex;
		_IndexToEntity[newIndex] = entity;
		_Components[newIndex] = std::move(component);

		++m_Size;
	}

	inline void Remove(Entity entity)
	{
		ASSERT(_EntityToIndex.find(entity) != _EntityToIndex.end());

		const size_t indexOfRemovedEntity = _EntityToIndex[entity];
		const size_t indexOfLastElement = m_Size - 1;
		_Components[indexOfRemovedEntity] = std::move(_Components[indexOfLastElement]);

		const Entity entityOfLastElement = _IndexToEntity[indexOfLastElement];
		_EntityToIndex[entityOfLastElement] = indexOfRemovedEntity;
		_IndexToEntity[indexOfRemovedEntity] = entityOfLastElement;

		_EntityToIndex.erase(entity);
		_IndexToEntity.erase(indexOfLastElement);

		--m_Size;
	}

	inline T& Get(Entity entity)
	{
		ASSERT(_EntityToIndex.find(entity) != _EntityToIndex.end());
		return _Components[_EntityToIndex[entity]];
	}

	inline void EntityDestroyed(Entity entity) override
	{
		ASSERT(_EntityToIndex.find(entity) != _EntityToIndex.end());
		Remove(entity);
	}

private:
	size_t m_Size = 0;
	std::array<T, GMaxEntities> _Components = {};
	std::unordered_map<Entity, size_t> _EntityToIndex = {};
	std::unordered_map<size_t, Entity> _IndexToEntity = {};
};
}
