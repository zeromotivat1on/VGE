#pragma once

#include "Common.h"

namespace vge 
{
	inline class Application* GApplication = nullptr;

	struct ApplicationSpecs
	{
		struct {
			const char* Name = nullptr;
			uint32_t Width = 0;
			uint32_t Height = 0;
		} Window;

		const char* Name = "";
		const char* InternalName = "";
	};

	class Application
	{
	public:
		Application(const ApplicationSpecs& specs);

		void Initialize();
		void Close();

	public:
		const ApplicationSpecs Specs = {};
	};

	int32_t Main(int argc, const char** argv);

	Application* CreateApplication(const ApplicationSpecs& specs);
	bool DestroyApplication();
}
