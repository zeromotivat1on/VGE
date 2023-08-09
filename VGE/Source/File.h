#pragma once

#include <vector>
#include <fstream>

namespace vge::file
{
	std::vector<char> ReadShader(const char* filename);
}
