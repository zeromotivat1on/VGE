#pragma once

#include "ComponentManager.h"
#include "EntityManager.h"
#include "SystemManager.h"

namespace vge::ecs
{
	inline class Coordinator* GCoordinator = nullptr;

	// General class for ECS usage.
	class Coordinator
	{
	public:
		Coordinator() = default;

		inline void Initialize()
		{
			m_ComponentManager = std::make_unique<ComponentManager>();
			m_EntityManager = std::make_unique<EntityManager>();
			m_SystemManager = std::make_unique<SystemManager>();
		}

		inline void Destroy() {}

	public:
		inline Entity CreateEntity()
		{
			return m_EntityManager->Create();
		}

		inline void DestroyEntity(Entity entity)
		{
			m_EntityManager->Destroy(entity);
			m_ComponentManager->EntityDestroyed(entity);
			m_SystemManager->EntityDestroyed(entity);
		}

	public:
		template<typename T>
		void RegisterComponent()
		{
			m_ComponentManager->RegisterComponent<T>();
		}

		template<typename T>
		void AddComponent(Entity entity, const T& component)
		{
			m_ComponentManager->Add<T>(entity, component);

			auto signature = m_EntityManager->GetSignature(entity);
			signature.set(m_ComponentManager->GetComponentType<T>(), true);
			m_EntityManager->SetSignature(entity, signature);

			m_SystemManager->EntitySignatureChanged(entity, signature);
		}

		template<typename T>
		void RemoveComponent(Entity entity)
		{
			m_ComponentManager->Remove<T>(entity);

			auto signature = m_EntityManager->GetSignature(entity);
			signature.set(m_ComponentManager->GetComponentType<T>(), false);
			m_EntityManager->SetSignature(entity, signature);

			m_SystemManager->EntitySignatureChanged(entity, signature);
		}

		template<typename T>
		T& GetComponent(Entity entity)
		{
			return m_ComponentManager->GetComponent<T>(entity);
		}

		template<typename T>
		ComponentType GetComponentType()
		{
			return m_ComponentManager->GetComponentType<T>();
		}


	public:
		template<typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			return m_SystemManager->RegisterSystem<T>();
		}

		template<typename T>
		void SetSystemSignature(Signature signature)
		{
			m_SystemManager->SetSignature<T>(signature);
		}

	private:
		std::unique_ptr<ComponentManager> m_ComponentManager = nullptr;
		std::unique_ptr<EntityManager> m_EntityManager = nullptr;
		std::unique_ptr<SystemManager> m_SystemManager = nullptr;
	};

	inline Coordinator* CreateCoordinator()
	{
		if (GCoordinator) return GCoordinator;
		return (GCoordinator = new Coordinator());
	}

	inline bool DestroyCoordinator()
	{
		if (!GCoordinator) return false;
		GCoordinator->Destroy();
		delete GCoordinator;
		GCoordinator = nullptr;
		return true;
	}

	inline Entity CreateEntity()
	{
		ENSURE(GCoordinator);
		return GCoordinator->CreateEntity();
	}

	inline void DestroyEntity(Entity entity)
	{
		ENSURE(GCoordinator);
		GCoordinator->DestroyEntity(entity);
	}

	template<typename T>
	inline void RegisterComponent()
	{
		ENSURE(GCoordinator);
		GCoordinator->RegisterComponent<T>();
	}

	template<typename T>
	inline void AddComponent(Entity entity, const T& component)
	{
		ENSURE(GCoordinator);
		GCoordinator->AddComponent(entity, component);
	}

	template<typename T>
	inline void RemoveComponent(Entity entity)
	{
		ENSURE(GCoordinator);
		GCoordinator->RemoveComponent(entity);
	}

	template<typename T>
	inline T& GetComponent(Entity entity)
	{
		ENSURE(GCoordinator);
		return GCoordinator->GetComponent<T>(entity);
	}

	template<typename T>
	inline ComponentType GetComponentType()
	{
		ENSURE(GCoordinator);
		return GCoordinator->GetComponentType<T>();
	}

	template<typename T>
	inline std::shared_ptr<T> RegisterSystem()
	{
		ENSURE(GCoordinator);
		return GCoordinator->RegisterSystem<T>();
	}

	template<typename T>
	inline void SetSystemSignature(Signature signature)
	{
		ENSURE(GCoordinator);
		return GCoordinator->SetSystemSignature<T>(signature);
	}
}
