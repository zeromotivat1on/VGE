#include "Context.h"
#include "Render/Core/Error.h"

namespace vge
{
// Converts wstring to string using Windows specific function.
inline std::string WstrToStr(const std::wstring &wstr)
{
    if (wstr.empty())
    {
        return {};
    }

    auto wstrLen = static_cast<int>(wstr.size());
    auto strLen  = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstrLen, NULL, 0, NULL, NULL);

    std::string str(strLen, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstrLen, &str[0], strLen, NULL, NULL);

    return str;
}

inline std::string GetTempPathFromEnvironment()
{
    std::string tempPath = "temp/";

    TCHAR tempBuffer[MAX_PATH];
    DWORD tempPathRet = GetTempPath(MAX_PATH, tempBuffer);
    if (tempPathRet > MAX_PATH || tempPathRet == 0)
    {
        tempPath = "temp/";
    }
    else
    {
        tempPath = WstrToStr(tempBuffer) + "/";
    }

    return tempPath;
}
    
inline std::vector<std::string> GetArgs()
{
    LPWSTR *argv;
    int argc;

    argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    // Ignore the first argument containing the application full path.
    std::vector<std::wstring> argStrings(argv + 1, argv + argc);
    std::vector<std::string> args;

    for (auto &arg : argStrings)
    {
        args.push_back(WstrToStr(arg));
    }

    return args;
}
}   // namespace vkb

vge::WindowsPlatformContext::WindowsPlatformContext(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
    : PlatformContext()
{
    _ExternalStorageDirectory = "";
    _TempDirectory = GetTempPathFromEnvironment();
    _Arguments = GetArgs();

    // Attempt to attach to the parent process console if it exists.
    //if (!AttachConsole(ATTACH_PARENT_PROCESS))
    //{
    //    // No parent console, allocate a new one for this process.
    //    ENSURE(AllocConsole());
    //}

    FILE *fp;
    freopen_s(&fp, "conin$", "r", stdin);
    freopen_s(&fp, "conout$", "w", stdout);
    freopen_s(&fp, "conout$", "w", stderr);
}
