#include "EntityManager.h"
#include "Entity.h"

vge::EntityManager::EntityManager()
{
	for (Entity entity = 0; entity < GMaxEntities; ++entity)
	{
		m_AvailableEntities.push(entity);
	}
}

vge::Entity vge::EntityManager::Create()
{
	ASSERT(m_LivingEntityCount < GMaxEntities);

	Entity id = m_AvailableEntities.front();
	m_AvailableEntities.pop();
	++m_LivingEntityCount;

	return id;
}

void vge::EntityManager::Destroy(Entity entity)
{
	ASSERT(entity < GMaxEntities);

	m_Signatures[entity].reset();

	m_AvailableEntities.push(entity);
	--m_LivingEntityCount;
}
