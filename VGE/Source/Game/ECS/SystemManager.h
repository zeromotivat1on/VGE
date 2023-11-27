#pragma once

#include "Common.h"
#include "System.h"
#include "Entity.h"
#include "Component.h"

namespace vge
{
	class SystemManager
	{
	public:
		SystemManager() = default;

		template<typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			const char* typeName = typeid(T).name();
			ASSERT(m_Systems.find(typeName) == m_Systems.end());

			auto system = std::make_shared<T>();
			m_Systems.insert({ typeName, system });

			return system;
		}

		template<typename T>
		void SetSignature(Signature signature)
		{
			const char* typeName = typeid(T).name();
			ASSERT(m_Systems.find(typeName) != m_Systems.end());
			m_Signatures.insert({ typeName, signature });
		}

		inline void EntityDestroyed(Entity entity)
		{
			for (const auto& pair : m_Systems)
			{
				const auto& system = pair.second;
				system->Remove(entity);
			}
		}

		inline void EntitySignatureChanged(Entity entity, Signature entitySignature)
		{
			for (const auto& pair : m_Systems)
			{
				const auto& type = pair.first;
				const auto& system = pair.second;
				const auto& systemSignature = m_Signatures[type];

				if ((entitySignature & systemSignature) == systemSignature)
				{
					system->Add(entity);
				}
				else
				{
					system->Remove(entity);
				}
			}
		}

	private:
		std::unordered_map<const char*, Signature> m_Signatures = {};
		std::unordered_map<const char*, std::shared_ptr<System>> m_Systems = {};
	};
}
