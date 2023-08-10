#include "File.h"
#include "Logging.h"

std::vector<char> vge::file::ReadShader(const char* filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file.is_open())
	{
		LOG(Error, "Failed to open a file %s.", filename);
		return {};
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> fileBuffer(fileSize);

	file.seekg(0);
	file.read(fileBuffer.data(), fileSize);

	file.close();

	return fileBuffer;
}
