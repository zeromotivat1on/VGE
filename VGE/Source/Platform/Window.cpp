#include "Window.h"

vge::Window::Window(const Properties& properties)
	: _Properties(properties)
{
}

vge::Window::Extent vge::Window::Resize(const Extent& extent)
{
	if (_Properties.Resizable)
	{
		_Properties.Extent.width = extent.width;
		_Properties.Extent.height = extent.height;
	}

	return _Properties.Extent;
}
