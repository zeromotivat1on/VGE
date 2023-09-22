#pragma once

#include "Common.h"
#include "RenderCommon.h"

namespace vge
{
	class Device;

	class CmdBuffer
	{
		
	};

	VkCommandBuffer BeginOneTimeCmdBuffer(const Device* device);
	void EndOneTimeCmdBuffer(const Device* device, VkCommandBuffer cmdBuffer);

	// Simple wrapper for BeginOneTimeCmdBuffer (ctor) and EndOneTimeCmdBuffer (dtor) functions.
	struct ScopeCmdBuffer
	{
	public:
		ScopeCmdBuffer(const Device* device);
		~ScopeCmdBuffer();

		VkCommandBuffer Get() const { return m_CmdBuffer; }

	private:
		const Device* m_Device = nullptr;
		VkCommandBuffer m_CmdBuffer = VK_NULL_HANDLE;
	};
}

