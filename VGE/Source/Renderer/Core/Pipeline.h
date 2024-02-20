#pragma once

#include "Core/Common.h"
#include "Core/PipelineState.h"

namespace vge
{
class Device;

class Pipeline
{
public:
	Pipeline(Device& device);

	COPY_CTOR_DEL(Pipeline);
	Pipeline(Pipeline&& other);

	virtual ~Pipeline();

	COPY_OP_DEL(Pipeline);
	MOVE_OP_DEL(Pipeline);

public:
	inline VkPipeline get_handle() const { return handle; }
	inline const PipelineState& get_state() const { return state; }

protected:
	Device& device;
	VkPipeline handle = VK_NULL_HANDLE;
	PipelineState state;
};

class ComputePipeline : public Pipeline
{
public:
	ComputePipeline(Device&, VkPipelineCache, PipelineState&);
	MOVE_CTOR_DEF(ComputePipeline);

	virtual ~ComputePipeline() = default;
};

class GraphicsPipeline : public Pipeline
{
public:
	GraphicsPipeline(Device&, VkPipelineCache, PipelineState&);
	MOVE_CTOR_DEF(GraphicsPipeline);

	virtual ~GraphicsPipeline() = default;
};
}	// namespace vge
