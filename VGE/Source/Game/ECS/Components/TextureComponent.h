#pragma once

#include <string>

namespace vge
{
struct ImageComponent;
struct SamplerComponent;

struct TextureComponent
{
	std::string Name;
	ImageComponent* Image = nullptr;
	SamplerComponent* Sampler = nullptr;
};
}	// namespace vge
