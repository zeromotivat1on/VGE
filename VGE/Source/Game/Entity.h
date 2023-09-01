#pragma once

#include "Common.h"

namespace vge
{
	class Entity
	{
	public:
		Entity() = default;

		void Initialize(uint32 Id);

	private:
		uint32 m_Id = INDEX_NONE;
	};
}
