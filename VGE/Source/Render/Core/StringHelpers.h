#pragma once

#include <vector>
#include <string>
#include <sstream>

namespace vge
{
std::vector<std::string> Split(const std::string& input, char delim)
{
	std::vector<std::string> tokens;

	std::stringstream sstream(input);
	std::string token;
	while (std::getline(sstream, token, delim))
	{
		tokens.push_back(token);
	}

	return tokens;
}
}	// namespace vge