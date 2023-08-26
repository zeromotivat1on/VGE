#pragma once

#include "Common.h"

namespace vge 
{
	inline class Application* GApplication = nullptr;

	inline int32 GAppFrame = 0;

	struct ApplicationSpecs
	{
		struct {
			const char* Name = nullptr;
			uint32 Width = 0;
			uint32 Height = 0;
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

		bool ShouldClose() const;

	public:
		const ApplicationSpecs Specs = {};
	};

	Application* CreateApplication(const ApplicationSpecs& specs);
	bool DestroyApplication();

	int32 Main(int argc, const char** argv);

	inline void IncrementAppFrame() { ++GAppFrame; }
}
