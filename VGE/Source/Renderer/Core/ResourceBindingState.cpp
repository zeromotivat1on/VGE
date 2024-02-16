#include "ResourceBindingState.h"

void vge::ResourceBindingState::BindBuffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, u32 set, u32 binding, u32 arrayElement)
{
	_ResourceSets[set].BindBuffer(buffer, offset, range, binding, arrayElement);
	_Dirty = true;
}

void vge::ResourceBindingState::BindImage(const ImageView& imageView, const Sampler& sampler, u32 set, u32 binding, u32 arrayElement)
{
	_ResourceSets[set].BindImage(imageView, sampler, binding, arrayElement);
	_Dirty = true;
}

void vge::ResourceBindingState::BindImage(const ImageView& imageView, u32 set, u32 binding, u32 arrayElement)
{
	_ResourceSets[set].BindImage(imageView, binding, arrayElement);
	_Dirty = true;
}

void vge::ResourceBindingState::BindInput(const ImageView& imageView, u32 set, u32 binding, u32 arrayElement)
{
	_ResourceSets[set].BindInput(imageView, binding, arrayElement);
	_Dirty = true;
}

void vge::ResourceSet::BindBuffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, u32 binding, u32 arrayElement)
{
	_ResourceBindings[binding][arrayElement].Dirty = true;
	_ResourceBindings[binding][arrayElement].Buffer = &buffer;
	_ResourceBindings[binding][arrayElement].Offset = offset;
	_ResourceBindings[binding][arrayElement].Range = range;

	_Dirty = true;
}

void vge::ResourceSet::BindImage(const ImageView& imageView, const Sampler& sampler, u32 binding, u32 arrayElement)
{
	_ResourceBindings[binding][arrayElement].Dirty = true;
	_ResourceBindings[binding][arrayElement].ImageView = &imageView;
	_ResourceBindings[binding][arrayElement].Sampler = &sampler;

	_Dirty = true;
}

void vge::ResourceSet::BindImage(const ImageView& imageView, u32 binding, u32 arrayElement)
{
	_ResourceBindings[binding][arrayElement].Dirty = true;
	_ResourceBindings[binding][arrayElement].ImageView = &imageView;
	_ResourceBindings[binding][arrayElement].Sampler = nullptr;

	_Dirty = true;
}

void vge::ResourceSet::BindInput(const ImageView& imageView, u32 binding, u32 arrayElement)
{
	_ResourceBindings[binding][arrayElement].Dirty = true;
	_ResourceBindings[binding][arrayElement].ImageView = &imageView;

	_Dirty = true;
}
