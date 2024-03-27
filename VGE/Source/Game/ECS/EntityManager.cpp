#include "EntityManager.h"
#include "Entity.h"

vge::ecs::EntityManager::EntityManager()
{
	for (Entity entity = 0; entity < GMaxEntities; ++entity)
	{
		m_AvailableEntities.push(entity);
	}
}

vge::Entity vge::ecs::EntityManager::Create()
{
	ASSERT(m_LivingEntityCount < GMaxEntities);

	Entity id = m_AvailableEntities.front();
	m_AvailableEntities.pop();
	++m_LivingEntityCount;

	return id;
}

void vge::ecs::EntityManager::Destroy(Entity entity)
{
	ASSERT(entity < GMaxEntities);

	m_Signatures[entity].reset();

	m_AvailableEntities.push(entity);
	--m_LivingEntityCount;
}
