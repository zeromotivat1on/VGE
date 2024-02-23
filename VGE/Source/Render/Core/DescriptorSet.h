#pragma once

#include "VkCommon.h"

namespace vge
{
class Device;
class DescriptorSetLayout;
class DescriptorPool;

// A descriptor set handle allocated from a \ref DescriptorPool.
// Destroying the handle has no effect, as the pool manages the lifecycle of its descriptor sets.
// Keeps track of what bindings were written to prevent a double write.
class DescriptorSet
{
public:
	DescriptorSet(
		Device& device,
		const DescriptorSetLayout& descriptorSetLayout,
		DescriptorPool& descriptorPool,
		const BindingMap<VkDescriptorBufferInfo>& bufferInfos = {},
		const BindingMap<VkDescriptorImageInfo>& imageInfos = {});

	COPY_CTOR_DEL(DescriptorSet);
	DescriptorSet(DescriptorSet&& other);

	// The descriptor set handle is managed by the pool and will be destroyed when the pool is reset.
	~DescriptorSet() = default;

	COPY_OP_DEL(DescriptorSet);
	MOVE_OP_DEL(DescriptorSet);

public:
	inline const DescriptorSetLayout& GetLayout() const { return _DescriptorSetLayout; }
	inline VkDescriptorSet GetHandle() const { return _Handle; }
	inline BindingMap<VkDescriptorBufferInfo>& GetBufferInfos() { return _BufferInfos; }
	inline BindingMap<VkDescriptorImageInfo>& GetImageInfos() { return _ImageInfos; }

	// Resets the DescriptorSet state. Optionally prepares a new set of buffer infos and/or image infos.
	void Reset(const BindingMap<VkDescriptorBufferInfo>& newBufferInfos = {}, const BindingMap<VkDescriptorImageInfo>& newImageInfos = {});

	// Updates the contents of the DescriptorSet by performing the write operations
	void Update(const std::vector<uint32_t>& bindingsToUpdate = {});

	// Applies pending write operations without updating the state
	void ApplyWrites() const;

protected:
	// Prepares the descriptor set to have its contents updated by loading a vector of write operations.
	// Cannot be called twice during the lifetime of a DescriptorSet.
	void Prepare();

private:
	Device& _Device;
	VkDescriptorSet _Handle = VK_NULL_HANDLE;
	DescriptorPool& _DescriptorPool;
	const DescriptorSetLayout& _DescriptorSetLayout;
	BindingMap<VkDescriptorBufferInfo> _BufferInfos;
	BindingMap<VkDescriptorImageInfo> _ImageInfos;
	std::vector<VkWriteDescriptorSet> _WriteDescriptorSets; // the list of write operations for the descriptor set
	// The bindings of the write descriptors that have had vkUpdateDescriptorSets since the last call to update().
	// Each binding number is mapped to a hash of the binding description that it will be updated to.
	std::unordered_map<uint32_t, size_t> _UpdatedBindings;
};
}	// namespace vge
