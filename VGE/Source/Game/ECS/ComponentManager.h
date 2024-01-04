#pragma once

#include "Common.h"
#include "Component.h"
#include "ComponentArray.h"
#include "Entity.h"

namespace vge
{
	class ComponentManager
	{
	public:
		ComponentManager() = default;

		template<typename T>
		void RegisterComponent()
		{
			const char* typeName = typeid(T).name();
			ASSERT(m_ComponentTypes.find(typeName) == m_ComponentTypes.end());

			m_ComponentTypes.insert({ typeName, m_NextComponentType });
			m_ComponentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });

			++m_NextComponentType;
		}

		template<typename T>
		ComponentType GetComponentType()
		{
			const char* typeName = typeid(T).name();
			ASSERT(m_ComponentTypes.find(typeName) != m_ComponentTypes.end());
			return m_ComponentTypes[typeName];
		}

		template<typename T>
		void Add(Entity entity, const T& component)
		{
			GetComponentArray<T>()->Add(entity, component);
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
		std::unordered_map<const char*, ComponentType> m_ComponentTypes = {};
		std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_ComponentArrays = {};

		template<typename T>
		std::shared_ptr<ComponentArray<T>> GetComponentArray()
		{
			const char* typeName = typeid(T).name();
			if (m_ComponentTypes.find(typeName) != m_ComponentTypes.end())
			{
				return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays[typeName]);
			}

			return nullptr;
		}
	};
}
