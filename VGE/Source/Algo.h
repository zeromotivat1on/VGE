#pragma once

namespace vge::algo
{
	template <typename Container, typename T>
	const T* Find(const Container& container, const T& elem)
	{
		if (auto it = std::find(std::cbegin(container), std::cend(container), elem); it != std::cend(container))
		{
			return &(*it);
		}

		return nullptr;
	}

	template <typename Container, typename T>
	T* FindMutable(Container& container, const T& elem)
	{
		if (auto it = std::find(std::begin(container), std::end(container), elem); it != std::end(container))
		{
			return &(*it);
		}

		return nullptr;
	}
}
