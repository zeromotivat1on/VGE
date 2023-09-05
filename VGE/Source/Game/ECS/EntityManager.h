#pragma once

#include "Common.h"
#include "Entity.h"
#include "ECS/Component.h"

namespace vge
{
	class EntityManager
	{
	public:
		EntityManager();

		Entity Create();
		void Destroy(Entity entity);
		
		inline Signature GetSignature(Entity entity)
		{
			ASSERT(entity < GMaxEntities);
			return m_Signatures[entity];
		}

		inline void SetSignature(Entity entity, Signature signature)
		{
			ASSERT(entity < GMaxEntities);
			m_Signatures[entity] = signature;
		}

	private:
		uint32_t m_LivingEntityCount = 0;
		std::queue<Entity> m_AvailableEntities = {};
		std::array<Signature, GMaxEntities> m_Signatures = {};
	};
}
