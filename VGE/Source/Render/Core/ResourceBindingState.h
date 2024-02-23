#pragma once

#include "VkCommon.h"
#include "Core/Buffer.h"
#include "Core/Sampler.h"
#include "Core/ImageView.h"

namespace vge
{
/**
	* @brief A resource info is a struct containing the actual resource data.
	*
	* This will be referenced by a buffer info or image info descriptor inside a descriptor set.
	*/
struct ResourceInfo
{
	bool Dirty = false;
	const Buffer* Buffer = nullptr;
	VkDeviceSize Offset = 0;
	VkDeviceSize Range = 0;
	const ImageView* ImageView = nullptr;
	const Sampler* Sampler = nullptr;
};

/**
	* @brief A resource set is a set of bindings containing resources that were bound
	*        by a command buffer.
	*
	* The ResourceSet has a one to one mapping with a DescriptorSet.
	*/
class ResourceSet
{
public:
	inline bool IsDirty() const { return _Dirty; }
	inline const BindingMap<ResourceInfo>& GetResourceBindings() const { return _ResourceBindings; }

	inline void Reset() { ClearDirty(); _ResourceBindings.clear(); }
	inline void ClearDirty() { _Dirty = false; }
	inline void ClearDirty(u32 binding, u32 arrayElement) { _ResourceBindings[binding][arrayElement].Dirty = false; }

	void BindBuffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, u32 binding, u32 arrayElement);
	void BindImage(const ImageView& imageView, const Sampler& sampler, u32 binding, u32 arrayElement);
	void BindImage(const ImageView& imageView, u32 binding, u32 arrayElement);
	void BindInput(const ImageView& imageView, u32 binding, u32 arrayElement);

private:
	bool _Dirty = false;
	BindingMap<ResourceInfo> _ResourceBindings;
};

/**
	* @brief The resource binding state of a command buffer.
	*
	* Keeps track of all the resources bound by the command buffer. The ResourceBindingState is used by
	* the command buffer to create the appropriate descriptor sets when it comes to draw.
	*/
class ResourceBindingState
{
public:
	inline bool IsDirty() const { return _Dirty; }
	inline const std::unordered_map<u32, ResourceSet>& GetResourceSets() const { return _ResourceSets; }
	
	inline void Reset() { ClearDirty(); _ResourceSets.clear(); }
	inline void ClearDirty() { _Dirty = false; }
	inline void ClearDirty(u32 set) { _ResourceSets[set].ClearDirty(); }

	void BindBuffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, u32 set, u32 binding, u32 arrayElement);
	void BindImage(const ImageView& imageView, const Sampler& sampler, u32 set, u32 binding, u32 arrayElement);
	void BindImage(const ImageView& imageView, u32 set, u32 binding, u32 arrayElement);
	void BindInput(const ImageView& imageView, u32 set, u32 binding, u32 arrayElement);

private:
	bool _Dirty = false;
	std::unordered_map<u32, ResourceSet> _ResourceSets;
};
}	// namespace vge
