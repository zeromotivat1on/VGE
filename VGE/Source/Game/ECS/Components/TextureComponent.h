#pragma once

#include <string>
#include "ECS/Component.h"

namespace vge
{
struct ImageComponent;
struct SamplerComponent;

struct TextureComponent : public Component
{
	std::string Name;
	ImageComponent* Image = nullptr;
	SamplerComponent* Sampler = nullptr;
};
}	// namespace vge
