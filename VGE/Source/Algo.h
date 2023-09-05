#pragma once

namespace vge::algo
{
	template <typename Container, typename T>
	T* Find(Container& container, const T& elem)
	{
		if (auto it = std::find(std::begin(container), std::end(container), elem); it != std::end(container))
		{
			return &(*it);
		}

		return nullptr;
	}

	template <typename Container, typename T>
	const T* Find(const Container& container, const T& elem)
	{
		if (auto it = std::find(std::begin(container), std::end(container), elem); it != std::end(container))
		{
			return &(*it);
		}

		return nullptr;
	}
}
