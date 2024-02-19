#include "Window.h"

vge::Window::Window(const Properties& properties)
	: _Properties(properties)
{
}

vge::Window::Extent vge::Window::Resize(const Extent& extent)
{
	if (_Properties.resizable)
	{
		_Properties.extent.width = extent.width;
		_Properties.extent.height = extent.height;
	}

	return _Properties.extent;
}
