#pragma once

#include "Common.h"
#include "EngineLoop.h"

namespace vge 
{
	inline class Application* GApplication = nullptr;

	inline uint64 GAppFrame = 0;

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
		NOT_COPYABLE(Application);

	public:
		void Initialize();
		void Run();
		void Close();

		bool ShouldClose() const;

	public:
		const ApplicationSpecs Specs = {};

	private:
		EngineLoop m_EngineLoop = {};
	};

	inline Application* CreateApplication(const ApplicationSpecs& specs)
	{
		if (GApplication) return GApplication;
		return (GApplication = new Application(specs));
	}

	inline bool DestroyApplication()
	{
		if (!GApplication) return false;
		GApplication->Close();
		delete GApplication;
		GApplication = nullptr;
		return true;
	}

	int32 Main(int argc, const char** argv);
}
