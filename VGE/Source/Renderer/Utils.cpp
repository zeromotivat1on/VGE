#include "Utils.h"
#include "Renderer.h"
#include "Texture.h"
#include "Window.h"
#include "Buffer.h"
#include "File.h"

const vge::c8* vge::GpuTypeToString(VkPhysicalDeviceType gpuType)
{
	switch (gpuType)
	{
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		return "Integrated";
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		return "Discrete";
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		return "Virtual";
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		return "CPU";
	default:
		return "Other";
	}
}

vge::u32 vge::FindMemoryTypeIndex(VkPhysicalDevice gpu, u32 allowedTypes, VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProperties);

	for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		const bool typeIsAllowed = allowedTypes & (1 << i);
		const bool propsHaveGivenFlags = (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags;

		if (typeIsAllowed && propsHaveGivenFlags)
		{
			return i;
		}
	}

	LOG(Warning, "Failed to find memory type index. Returning 0.");
	return 0;
}

void vge::CreateTextureDescriptorSet(VkDevice device, VkSampler sampler, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descrptorSetLayout, VkImageView textureImageView, VkDescriptorSet& outTextureDescriptorSet)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descrptorSetLayout;

	VK_ENSURE_MSG(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &outTextureDescriptorSet), "Failed to allocate texture descriptor set.");

	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfo.imageView = textureImageView;
	descriptorImageInfo.sampler = sampler;

	VkWriteDescriptorSet descriptorSetWrite = {};
	descriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSetWrite.dstSet = outTextureDescriptorSet;
	descriptorSetWrite.dstBinding = 0;
	descriptorSetWrite.dstArrayElement = 0;
	descriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSetWrite.descriptorCount = 1;
	descriptorSetWrite.pImageInfo = &descriptorImageInfo;

	vkUpdateDescriptorSets(device, 1, &descriptorSetWrite, 0, nullptr);
}

void vge::GetTexturesFromMaterials(const aiScene* scene, std::vector<const c8*>& outTextures)
{
	outTextures.resize(scene->mNumMaterials, "");

	for (u32 i = 0; i < scene->mNumMaterials; ++i)
	{
		const aiMaterial* material = scene->mMaterials[i];

		if (!material)
		{
			continue;
		}

		// TODO: add possibility to load different textures.
		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;
			// TODO: retreive all textures from material.
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				const i32 LastSlashIndex = static_cast<i32>(std::string(path.data).rfind("\\"));
				const std::string filename = std::string(path.data).substr(LastSlashIndex + 1);
				outTextures[i] = filename.c_str();
			}
		}
	}
}

void vge::ResolveTexturesForDescriptors(Renderer* renderer, const std::vector<const c8*>& texturePaths, std::vector<i32>& outTextureToDescriptorSet)
{
	if (!renderer)
	{
		return;
	}

	outTextureToDescriptorSet.resize(texturePaths.size());

	for (size_t i = 0; i < texturePaths.size(); ++i)
	{
		if (texturePaths[i] == "")
		{
			outTextureToDescriptorSet[i] = 0;
		}
		else
		{
			outTextureToDescriptorSet[i] = renderer->CreateTexture(texturePaths[i]);
		}
	}
}
