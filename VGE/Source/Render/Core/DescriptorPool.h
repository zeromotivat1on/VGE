#pragma once

#include "VkCommon.h"
#include "Core/Error.h"

namespace vge
{
class Device;
class DescriptorSetLayout;

// Manages an array of fixed size VkDescriptorPool and is able to allocate descriptor sets
class DescriptorPool
{
public:
	static const u32 MaxSetsPerPool = 16;

public:
	DescriptorPool(Device& device, const DescriptorSetLayout& descriptorSetLayout, u32 poolSize = MaxSetsPerPool);

	COPY_CTOR_DEL(DescriptorPool);
	MOVE_CTOR_DEF(DescriptorPool);

	~DescriptorPool();

	COPY_OP_DEL(DescriptorPool);
	MOVE_OP_DEL(DescriptorPool);

public:
	inline const DescriptorSetLayout& GetDescriptorSetLayout() const { ASSERT(_DescriptorSetLayout); return *_DescriptorSetLayout; }
	inline void SetDescriptorSetLayout(const DescriptorSetLayout& setLayout) { _DescriptorSetLayout = &setLayout; }

	void Reset();
	VkDescriptorSet Allocate();
	VkResult Free(VkDescriptorSet descriptorSet);

private:
	u32 FindAvailablePool(u32 poolIndex); // find next pool index or create new pool

private:
	Device& _Device;
	const DescriptorSetLayout* _DescriptorSetLayout = nullptr;
	std::vector<VkDescriptorPoolSize> _PoolSizes;
	u32 _PoolMaxSets = 0; // number of sets to allocate for each pool
	std::vector<VkDescriptorPool> _Pools; // total descriptor pools created
	std::vector<u32> _PoolSetsCount; // count sets for each pool
	u32 _PoolIndex = 0; // current pool index to allocate descriptor set
	std::unordered_map<VkDescriptorSet, u32> _SetPoolMapping; // map between descriptor set and pool index
};
}	// namespace vge
