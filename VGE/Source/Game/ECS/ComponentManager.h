#pragma once

#include <typeindex>

#include "Common.h"
#include "Component.h"
#include "ComponentArray.h"
#include "Entity.h"
#include "Core/Error.h"

namespace vge::ecs
{
class ComponentManager
{
public:
	ComponentManager() = default;

	template<typename T>
	size_t GetComponentsNum() const { return GetComponentArray<T>()->GetSize(); }
	
	template<typename T>
	T* GetComponents() { return GetComponentArray<T>()->GetComponents(); }
	
	template<typename T>
	void RegisterComponent()
	{
		const auto typeIndex = std::type_index(typeid(T));
		ASSERT(m_ComponentTypes.find(typeIndex) == m_ComponentTypes.end());

		m_ComponentTypes.insert({ typeIndex, m_NextComponentType });
		m_ComponentArrays.insert({ typeIndex, std::make_shared<ComponentArray<T>>() });

		++m_NextComponentType;
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		const auto typeIndex = std::type_index(typeid(T));
		ASSERT(m_ComponentTypes.find(typeIndex) != m_ComponentTypes.end());
		return m_ComponentTypes[typeIndex];
	}

	template<typename T>
	void Add(Entity entity, T&& component)
	{
		GetComponentArray<T>()->Add(entity, std::forward<T>(component));
	}

	template<typename T>
	void Remove(Entity entity)
	{
		GetComponentArray<T>()->Remove(entity);
	}

	template<typename T>
	T& GetComponent(Entity entity)
	{
		return GetComponentArray<T>()->Get(entity);
	}

	void EntityDestroyed(Entity entity)
	{
		for (const auto& pair : m_ComponentArrays)
		{
			const auto& component = pair.second;
			component->EntityDestroyed(entity);
		}
	}

private:
	ComponentType m_NextComponentType = {};
	std::unordered_map<std::type_index, ComponentType> m_ComponentTypes = {};
	std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_ComponentArrays = {};

	template<typename T>
	std::shared_ptr<ComponentArray<T>> GetComponentArray()
	{
		const auto typeIndex = std::type_index(typeid(T));
		if (m_ComponentTypes.find(typeIndex) != m_ComponentTypes.end())
		{
			return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays[typeIndex]);
		}

		return nullptr;
	}
};
}
