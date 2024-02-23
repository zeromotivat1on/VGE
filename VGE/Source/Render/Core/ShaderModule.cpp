#include "ShaderModule.h"
#include "File.h"
#include "Core/StringHelpers.h"

/**
	* @brief Pre-compiles project shader files to include header code
	* @param source The shader file
	* @returns A byte array of the final shader
	*/
namespace vge
{
std::vector<std::string> PrecompileShader(const std::string& source)
{
	std::vector<std::string> finalFile;

	auto lines = Split(source, '\n');

	for (auto& line : lines)
	{
		if (line.find("#include \"") == 0)
		{
			// Include paths are relative to the base shader directory
			std::string includePath = line.substr(10);
			size_t lastQuote = includePath.find("\"");
			if (!includePath.empty() && lastQuote != std::string::npos)
			{
				includePath = includePath.substr(0, lastQuote);
			}

			auto includeFile = PrecompileShader(file::ReadTextFile(includePath.c_str()));
			for (auto& includeFileLine : includeFile)
			{
				finalFile.push_back(includeFileLine);
			}
		}
		else
		{
			finalFile.push_back(line);
		}
	}

	return finalFile;
}

std::vector<u8> ToBytes(std::vector<std::string>& lines)
{
	std::vector<u8> bytes;

	for (auto& line : lines)
	{
		line += "\n";
		std::vector<u8> lineBytes(line.begin(), line.end());
		bytes.insert(bytes.end(), lineBytes.begin(), lineBytes.end());
	}

	return bytes;
}
}	// namespace vge

vge::ShaderModule::ShaderModule(
	Device& device, VkShaderStageFlagBits stage, const vge::ShaderSource& glslSource, const std::string& entryPoint, const vge::ShaderVariant& shaderVariant) 
	: _Device(device), _Stage(stage), _EntryPoint(entryPoint)
{
	_DebugName = glslSource.GetFilename();
		//fmt::format("{} [variant {:X}] [entrypoint {}]", glslSource.GetFilename(), shaderVariant.GetId(), entryPoint);

	// Compiling from GLSL source requires the entry point.
	ENSURE(!entryPoint.empty());

	auto& source = glslSource.GetSource();

	// Check if application is passing in GLSL source code to compile to SPIR-V.
	ENSURE(!source.empty());

	// Precompile source into the final spirv bytecode.
	auto glslFinalSource = PrecompileShader(source);

	// Compile the GLSL source.
	//GLSLCompiler glslCompiler;
	//if (!glslCompiler.compile_to_spirv(stage, ToBytes(glslFinalSource), entryPoint, shaderVariant, _Spirv, _InfoLog))
	//{
	//	LOG(Error, "Shader compilation failed for shader %s", glslSource.GetFilename());
	//	LOG(Error, "%s", _InfoLog);
	//	ENSURE_MSG(false, "Shader compilation failed, see errors above.");
	//}

	// Reflect all shader resources.
	//SPIRVReflection spirvReflection;
	//ENSURE(spirvReflection.reflect_shader_resources(stage, _Spirv, _Resources, shaderVariant));

	// Generate a unique id, determined by source and variant.
	std::hash<std::string> hasher = {};
	_Id = hasher(std::string(reinterpret_cast<const char*>(_Spirv.data()), reinterpret_cast<const char*>(_Spirv.data() + _Spirv.size())));
}

vge::ShaderModule::ShaderModule(vge::ShaderModule&& other) :
	_Device(other._Device),
	_Id(other._Id),
	_Stage(other._Stage),
	_EntryPoint(other._EntryPoint),
	_DebugName(other._DebugName),
	_Spirv(other._Spirv),
	_Resources(other._Resources),
	_InfoLog(other._InfoLog)
{
	other._Stage = {};
}

void vge::ShaderModule::SetResourceMode(const std::string& resourceName, const ShaderResourceMode& resourceMode)
{
	auto it = std::find_if(_Resources.begin(), _Resources.end(), [&resourceName](const ShaderResource& resource) { return resource.Name == resourceName; });

	if (it != _Resources.end())
	{
		if (resourceMode == ShaderResourceMode::Dynamic)
		{
			if (it->Type == ShaderResourceType::BufferUniform || it->Type == ShaderResourceType::BufferStorage)
			{
				it->Mode = resourceMode;
			}
			else
			{
				LOG(Warning, "Resource %s does not support dynamic.", resourceName);
			}
		}
		else
		{
			it->Mode = resourceMode;
		}
	}
	else
	{
		LOG(Warning, "Resource %s not found for shader.", resourceName);
	}
}

vge::ShaderVariant::ShaderVariant(std::string&& preamble, std::vector<std::string>&& processes) 
	: _Preamble(std::move(preamble)), _Processes(std::move(processes))
{
	UpdateId();
}

void vge::ShaderVariant::AddDefinitions(const std::vector<std::string>& definitions)
{
	for (auto& definition : definitions)
	{
		AddDefine(definition);
	}
}

void vge::ShaderVariant::AddDefine(const std::string& def)
{
	_Processes.push_back("D" + def);

	std::string tmpDef = def;

	// The "=" needs to turn into a space
	size_t posEqual = tmpDef.find_first_of("=");
	if (posEqual != std::string::npos)
	{
		tmpDef[posEqual] = ' ';
	}

	_Preamble.append("#define " + tmpDef + "\n");

	UpdateId();
}

void vge::ShaderVariant::AddUndefine(const std::string& undef)
{
	_Processes.push_back("U" + undef);
	_Preamble.append("#undef " + undef + "\n");

	UpdateId();
}

void vge::ShaderVariant::AddRuntimeArraySize(const std::string& runtimeArrayName, size_t size)
{
	if (_RuntimeArraySizes.find(runtimeArrayName) == _RuntimeArraySizes.end())
	{
		_RuntimeArraySizes.insert({ runtimeArrayName, size });
	}
	else
	{
		_RuntimeArraySizes[runtimeArrayName] = size;
	}
}

void vge::ShaderVariant::Clear()
{
	_Preamble.clear();
	_Processes.clear();
	_RuntimeArraySizes.clear();
	UpdateId();
}

void vge::ShaderVariant::UpdateId()
{
	std::hash<std::string> hasher = {};
	_Id = hasher(_Preamble);
}

vge::ShaderSource::ShaderSource(const std::string& filename) 
	: _Filename(filename), _Source(file::ReadTextFile(filename.c_str()))
{
	std::hash<std::string> hasher = {};
	_Id = hasher(std::string(_Source.cbegin(), _Source.cend()));
}

void vge::ShaderSource::SetSource(const std::string& source_)
{
	_Source = source_;
	std::hash<std::string> hasher{};
	_Id = hasher(std::string(_Source.cbegin(), _Source.cend()));
}
