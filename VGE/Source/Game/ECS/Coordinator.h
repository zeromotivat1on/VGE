#pragma once

#include "ComponentManager.h"
#include "EntityManager.h"
#include "SystemManager.h"
#include "Core/Error.h"

namespace vge::ecs
{
// General class for ECS usage.
class Coordinator
{
private:
	Coordinator()
	{
		_ComponentManager = std::make_unique<ComponentManager>();
		_EntityManager = std::make_unique<EntityManager>();
		_SystemManager = std::make_unique<SystemManager>();
	}

public:
	inline static Coordinator& Get()
	{
		static Coordinator coordinator;
		return coordinator;
	}

public:
	inline Entity CreateEntity()
	{
		return _EntityManager->Create();
	}

	inline void DestroyEntity(Entity entity)
	{
		_EntityManager->Destroy(entity);
		_ComponentManager->EntityDestroyed(entity);
		_SystemManager->EntityDestroyed(entity);
	}

public:
	template<typename T>
	size_t GetComponentsNum() const { return _ComponentManager->GetComponentsNum<T>(); }
		
	template<typename T>
	T* GetComponents() { return _ComponentManager->GetComponents<T>(); }
	
	template<typename T>
	void RegisterComponent()
	{
		_ComponentManager->RegisterComponent<T>();
	}

	template<typename T>
	void AddComponent(Entity entity, T&& component)
	{
		_ComponentManager->Add<T>(entity, std::forward<T>(component));

		auto signature = _EntityManager->GetSignature(entity);
		signature.set(_ComponentManager->GetComponentType<T>(), true);
		_EntityManager->SetSignature(entity, signature);

		_SystemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	void RemoveComponent(Entity entity)
	{
		_ComponentManager->Remove<T>(entity);

		auto signature = _EntityManager->GetSignature(entity);
		signature.set(_ComponentManager->GetComponentType<T>(), false);
		_EntityManager->SetSignature(entity, signature);

		_SystemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	T& GetComponent(Entity entity)
	{
		return _ComponentManager->GetComponent<T>(entity);
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		return _ComponentManager->GetComponentType<T>();
	}

public:
	template<typename T>
	std::shared_ptr<T> RegisterSystem()
	{
		return _SystemManager->RegisterSystem<T>();
	}

	template<typename T>
	void SetSystemSignature(Signature signature)
	{
		_SystemManager->SetSignature<T>(signature);
	}

private:
	std::unique_ptr<ComponentManager> _ComponentManager = nullptr;
	std::unique_ptr<EntityManager> _EntityManager = nullptr;
	std::unique_ptr<SystemManager> _SystemManager = nullptr;
};

inline Entity CreateEntity() { return Coordinator::Get().CreateEntity(); }
inline void DestroyEntity(Entity entity) { Coordinator::Get().DestroyEntity(entity); }

template<typename T>
size_t GetComponentsNum() { return Coordinator::Get().GetComponentsNum<T>(); }
		
template<typename T>
T* GetComponents() { return Coordinator::Get().GetComponents<T>(); }
	
template<typename T>
void RegisterComponent() { Coordinator::Get().RegisterComponent<T>(); }

template<typename T>
void AddComponent(Entity entity, T&& component) { Coordinator::Get().AddComponent<T>(entity, std::forward<T>(component)); }

template<typename T>
void RemoveComponent(Entity entity) { Coordinator::Get().RemoveComponent<T>(entity); }

template<typename T>
T& GetComponent(Entity entity) { return Coordinator::Get().GetComponent<T>(entity); }

template<typename T>
ComponentType GetComponentType() { return Coordinator::Get().GetComponentType<T>(); }

template<typename T>
std::shared_ptr<T> RegisterSystem() { return Coordinator::Get().RegisterSystem<T>(); }

template<typename T>
void SetSystemSignature(Signature signature) { return Coordinator::Get().SetSystemSignature<T>(signature); }
}
