#pragma once

#include "Common.h"

namespace vge 
{
	inline class Application* GApplication = nullptr;

	inline u64 GAppFrame = 0;

	struct ApplicationSpecs
	{
		struct {
			const c8* Name = nullptr;
			u32 Width = 0;
			u32 Height = 0;
		} Window;

		const c8* Name = "";
		const c8* InternalName = "";
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

	i32 Main(int argc, const c8** argv);
}
