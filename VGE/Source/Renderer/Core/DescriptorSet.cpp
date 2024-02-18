#include "DescriptorSet.h"
#include "Device.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "Common/ResourceCaching.h"

vge::DescriptorSet::DescriptorSet(
	Device& device,
	const DescriptorSetLayout& descriptorSetLayout,
	DescriptorPool& descriptorPool,
	const BindingMap<VkDescriptorBufferInfo>& bufferInfos,
	const BindingMap<VkDescriptorImageInfo>& imageInfos) 
	:
	_Device(device),
	_DescriptorSetLayout(descriptorSetLayout),
	_DescriptorPool(descriptorPool),
	_BufferInfos(bufferInfos),
	_ImageInfos(imageInfos),
	_Handle(descriptorPool.Allocate())
{
	Prepare();
}

vge::DescriptorSet::DescriptorSet(DescriptorSet&& other) 
	:
	_Device(other._Device),
	_DescriptorSetLayout(other._DescriptorSetLayout),
	_DescriptorPool(other._DescriptorPool),
	_BufferInfos(std::move(other._BufferInfos)),
	_ImageInfos(std::move(other._ImageInfos)),
	_Handle(other._Handle),
	_WriteDescriptorSets(std::move(other._WriteDescriptorSets)),
	_UpdatedBindings(std::move(other._UpdatedBindings))
{
	other._Handle = VK_NULL_HANDLE;
}

void vge::DescriptorSet::Reset(const BindingMap<VkDescriptorBufferInfo>& newBufferInfos, const BindingMap<VkDescriptorImageInfo>& newImageInfos)
{
	if (!newBufferInfos.empty() || !newImageInfos.empty())
	{
		_BufferInfos = newBufferInfos;
		_ImageInfos = newImageInfos;
	}
	else
	{
		LOG(Warning, "Calling reset on Descriptor Set with no new buffer infos and no new image infos.");
	}

	_WriteDescriptorSets.clear();
	_UpdatedBindings.clear();

	Prepare();
}

void vge::DescriptorSet::Update(const std::vector<uint32_t>& bindingsToUpdate)
{
	std::vector<VkWriteDescriptorSet> writeOperations;
	std::vector<size_t> writeOperationHashes;

	// If the 'bindings_to_update' vector is empty, we want to write to all the bindings
	// (but skipping all to-update bindings that haven't been written yet)
	if (bindingsToUpdate.empty())
	{
		for (size_t i = 0; i < _WriteDescriptorSets.size(); i++)
		{
			const auto& writeOperation = _WriteDescriptorSets[i];

			size_t write_operation_hash = 0;
			HashParam(write_operation_hash, writeOperation);

			auto updatePairIt = _UpdatedBindings.find(writeOperation.dstBinding);
			if (updatePairIt == _UpdatedBindings.end() || updatePairIt->second != write_operation_hash)
			{
				writeOperations.push_back(writeOperation);
				writeOperationHashes.push_back(write_operation_hash);
			}
		}
	}
	else
	{
		// Otherwise we want to update the binding indices present in the 'bindings_to_update' vector (again, skipping those to update but not updated yet).
		for (size_t i = 0; i < _WriteDescriptorSets.size(); i++)
		{
			const auto& writeOperation = _WriteDescriptorSets[i];

			if (std::find(bindingsToUpdate.begin(), bindingsToUpdate.end(), writeOperation.dstBinding) != bindingsToUpdate.end())
			{
				size_t writeOperationHash = 0;
				HashParam(writeOperationHash, writeOperation);

				auto updatePairIt = _UpdatedBindings.find(writeOperation.dstBinding);
				if (updatePairIt == _UpdatedBindings.end() || updatePairIt->second != writeOperationHash)
				{
					writeOperations.push_back(writeOperation);
					writeOperationHashes.push_back(writeOperationHash);
				}
			}
		}
	}

	// Perform the Vulkan call to update the DescriptorSet by executing the write operations.
	if (!writeOperations.empty())
	{
		vkUpdateDescriptorSets(_Device.GetHandle(), ToU32(writeOperations.size()), writeOperations.data(), 0, nullptr);
	}

	// Store the bindings from the write operations that were executed by vkUpdateDescriptorSets (and their hash) 
	// to prevent overwriting by future calls to Update().
	for (size_t i = 0; i < writeOperations.size(); i++)
	{
		_UpdatedBindings[writeOperations[i].dstBinding] = writeOperationHashes[i];
	}
}

void vge::DescriptorSet::ApplyWrites() const
{
	vkUpdateDescriptorSets(_Device.GetHandle(),
		ToU32(_WriteDescriptorSets.size()),
		_WriteDescriptorSets.data(),
		0,
		nullptr);
}

void vge::DescriptorSet::Prepare()
{
	// We don't want to prepare twice during the life cycle of a Descriptor Set.
	if (!_WriteDescriptorSets.empty())
	{
		LOG(Warning, "Trying to prepare a descriptor set that has already been prepared, skipping.");
		return;
	}

	// Iterate over all buffer bindings.
	for (auto& bindingIt : _BufferInfos)
	{
		auto bindingIndex = bindingIt.first;
		auto& bufferBindings = bindingIt.second;

		if (auto bindingInfo = _DescriptorSetLayout.GetLayoutBinding(bindingIndex))
		{
			// Iterate over all binding buffers in array.
			for (auto& elementIt : bufferBindings)
			{
				auto& bufferInfo = elementIt.second;

				const size_t uniformBufferRangeLimit = _Device.GetGpu().GetProperties().limits.maxUniformBufferRange;
				const size_t storageBufferRangeLimit = _Device.GetGpu().GetProperties().limits.maxStorageBufferRange;

				size_t bufferRangeLimit = static_cast<size_t>(bufferInfo.range);

				if ((bindingInfo->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || bindingInfo->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) && bufferRangeLimit > uniformBufferRangeLimit)
				{
					LOG(Error, "Set {} binding {} cannot be updated: buffer size {} exceeds the uniform buffer range limit {}", 
						_DescriptorSetLayout.GetIndex(), bindingIndex, bufferInfo.range, uniformBufferRangeLimit);
					
					bufferRangeLimit = uniformBufferRangeLimit;
				}
				else if ((bindingInfo->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || bindingInfo->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) && bufferRangeLimit > storageBufferRangeLimit)
				{
					LOG(Error, "Set {} binding {} cannot be updated: buffer size {} exceeds the storage buffer range limit {}", 
						_DescriptorSetLayout.GetIndex(), bindingIndex, bufferInfo.range, storageBufferRangeLimit);
					
					bufferRangeLimit = storageBufferRangeLimit;
				}

				// Clip the buffers range to the limit if one exists as otherwise we will receive a Vulkan validation error.
				bufferInfo.range = bufferRangeLimit;

				VkWriteDescriptorSet writeDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
				writeDescriptorSet.dstBinding = bindingIndex;
				writeDescriptorSet.descriptorType = bindingInfo->descriptorType;
				writeDescriptorSet.pBufferInfo = &bufferInfo;
				writeDescriptorSet.dstSet = _Handle;
				writeDescriptorSet.dstArrayElement = elementIt.first;
				writeDescriptorSet.descriptorCount = 1;

				_WriteDescriptorSets.push_back(writeDescriptorSet);
			}
		}
		else
		{
			LOG(Error, "Shader layout set does not use buffer binding at %d.", bindingIndex);
		}
	}

	// Iterate over all image bindings.
	for (auto& bindingIt : _ImageInfos)
	{
		auto bindingIndex = bindingIt.first;
		auto& bindingResources = bindingIt.second;

		if (auto bindingInfo = _DescriptorSetLayout.GetLayoutBinding(bindingIndex))
		{
			// Iterate over all binding images in array.
			for (auto& elementIt : bindingResources)
			{
				auto& imageInfo = elementIt.second;

				VkWriteDescriptorSet write_descriptor_set = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
				write_descriptor_set.dstBinding = bindingIndex;
				write_descriptor_set.descriptorType = bindingInfo->descriptorType;
				write_descriptor_set.pImageInfo = &imageInfo;
				write_descriptor_set.dstSet = _Handle;
				write_descriptor_set.dstArrayElement = elementIt.first;
				write_descriptor_set.descriptorCount = 1;

				_WriteDescriptorSets.push_back(write_descriptor_set);
			}
		}
		else
		{
			LOG(Error, "Shader layout set does not use image binding at %d.", bindingIndex);
		}
	}
}
