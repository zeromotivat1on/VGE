#pragma once

#include <string>
#include <vector>

namespace vge
{
class UnixPlatformContext;
class WindowsPlatformContext;
class AndroidPlatformContext;

/**
 * A platform context contains abstract platform specific operations
 *
 * A platform can be thought as the physical device and operating system configuration that the application
 * is running on.
 *
 * Some platforms can be reused across different hardware configurations, such as Linux and Macos as both
 * are POSIX compliant. However, some platforms are more specific such as Android and Windows
 */
class PlatformContext
{
    // Only allow platform contexts to be created by the platform specific implementations.
    friend class WindowsPlatformContext;
    //friend class UnixPlatformContext;
    //friend class AndroidPlatformContext;

public:
    virtual ~PlatformContext() = default;

    virtual const std::vector<std::string>& GetArguments() const { return _Arguments; }
    virtual const std::string& GetTempDirectory() const { return _TempDirectory; }
    virtual const std::string& GetExternalStorageDirectory() const { return _ExternalStorageDirectory; }

protected:
    PlatformContext() = default;

protected:
    std::vector<std::string> _Arguments;
    std::string _ExternalStorageDirectory;
    std::string _TempDirectory;
};
}   // namespace vge