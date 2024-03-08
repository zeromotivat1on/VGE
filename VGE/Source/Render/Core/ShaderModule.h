#pragma once

#include "VkCommon.h"

namespace vge
{
class Device;

enum class ShaderResourceType
{
	Input,
	InputAttachment,
	Output,
	Image,
	ImageSampler,
	ImageStorage,
	Sampler,
	BufferUniform,
	BufferStorage,
	PushConstant,
	SpecializationConstant,
	All
};

// This determines the type and method of how descriptor set should be created and bound.
enum class ShaderResourceMode
{
	Static,
	Dynamic,
	UpdateAfterBind
};

// A bitmask of qualifiers applied to a resource.
struct ShaderResourceQualifiers
{
	enum : u32
	{
		None = 0,
		NonReadable = 1,
		NonWritable = 2,
	};
};

// Store shader resource data, used by the shader module.
struct ShaderResource
{
	VkShaderStageFlags Stages;
	ShaderResourceType Type;
	ShaderResourceMode Mode;

	u32 Set;
	u32 Binding;
	u32 Location;
	u32 InputAttachmentIndex;
	u32 VecSize;
	u32 Columns;
	u32 ArraySize;
	u32 Offset;
	u32 Size;
	u32 ConstantId;
	u32 Qualifiers;

	std::string Name;
};

// Adds support for C style preprocessor macros to glsl shaders enabling you to define or undefine certain symbols.
class ShaderVariant
{
public:
	ShaderVariant() = default;

	ShaderVariant(std::string&& preamble, std::vector<std::string>&& processes);

public:
	inline size_t GetId() const { return _Id; }
	inline const std::string& GetPreamble() const { return _Preamble; }
	inline const std::vector<std::string>& GetProcesses() const { return _Processes; }
	inline const std::unordered_map<std::string, size_t>& GetRuntimeArraySizes() const { return _RuntimeArraySizes; }

	inline void SetRuntimeArraySizes(const std::unordered_map<std::string, size_t>& sizes) { _RuntimeArraySizes = sizes; }

	void AddDefinitions(const std::vector<std::string>& definitions); // add definitions to shader variant
	void AddDefine(const std::string& def); // adds a define macro to the shader
	void AddUndefine(const std::string& undef); // adds an undef macro to the shader

	// Specifies the size of a named runtime array for automatic reflection. If already specified, overrides the size.
	// @param runtimeArrayName String under which the runtime array is named in the shader
	// @param size Integer specifying the wanted size of the runtime array (in number of elements, not size in bytes), used for automatic allocation of buffers.
	void AddRuntimeArraySize(const std::string& runtimeArrayName, size_t size);

	void Clear();

private:
	void UpdateId();

private:
	size_t _Id;
	std::string _Preamble;
	std::vector<std::string> _Processes;
	std::unordered_map<std::string, size_t> _RuntimeArraySizes;
};

class ShaderSource
{
public:
	ShaderSource() = default;
	ShaderSource(const std::string& filename);

public:
	inline size_t GetId() const { return _Id; }
	inline const std::string& GetFilename() const { return _Filename; }
	inline const std::string& GetSource() const { return _Source; }

	void SetSource(const std::string& source);

private:
	size_t _Id;
	std::string _Filename;
	std::string _Source;
};

/**
* Contains shader code, with an entry point, for a specific shader stage.
* It is needed by a PipelineLayout to create a Pipeline.
* ShaderModule can do auto-pairing between shader code and textures.
* The low level code can change bindings, just keeping the name of the texture.
* Variants for each texture are also generated, such as HAS_BASE_COLOR_TEX.
* It works similarly for attribute locations. A current limitation is that only set 0
* is considered. Uniform buffers are currently hardcoded as well.
*/
class ShaderModule
{
public:
	ShaderModule(
		Device& device,
		VkShaderStageFlagBits stage,
		const ShaderSource& glslSource,
		const std::string& entryPoint,
		const ShaderVariant& shaderVariant);

	COPY_CTOR_DEL(ShaderModule);
	ShaderModule(ShaderModule&& other);

	COPY_OP_DEL(ShaderModule);
	MOVE_OP_DEL(ShaderModule);

public:
	inline size_t GetId() const { return _Id; }
	inline VkShaderStageFlagBits GetStage() const { return _Stage; }
	inline const std::string& GetEntryPoint() const { return _EntryPoint; }
	inline const std::vector<ShaderResource>& GetResources() const { return _Resources; }
	inline const std::string& GetInfoLog() const { return _InfoLog; }
	inline const std::vector<u32>& GetBinary() const { return _Spirv; }
	inline const std::string& GetDebugName() const { return _DebugName; }

	inline void SetDebugName(const std::string& name) { _DebugName = name; }

	/**
		* @brief Flags a resource to use a different method of being bound to the shader
		* @param resource_name The name of the shader resource
		* @param resource_mode The mode of how the shader resource will be bound
		*/
	void SetResourceMode(const std::string& resourceName, const ShaderResourceMode& resourceMode);

private:
	Device& _Device;
	size_t _Id; // shader unique id
	VkShaderStageFlagBits _Stage = {}; // stage of the shader (vertex, fragment, etc)
	std::string _EntryPoint; // name of the main function
	std::string _DebugName; // human-readable name for the shader
	std::vector<u32> _Spirv; // compiled source
	std::vector<ShaderResource> _Resources;
	std::string _InfoLog;
};
}	// namespace vge
