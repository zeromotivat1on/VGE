#pragma once

#include "VkCommon.h"
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
	inline VkPipeline GetHandle() const { return _Handle; }
	inline const PipelineState& GetState() const { return _State; }

protected:
	Device& _Device;
	VkPipeline _Handle = VK_NULL_HANDLE;
	PipelineState _State;
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
