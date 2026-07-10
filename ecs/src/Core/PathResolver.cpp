#include "Core/PathResolver.hpp"
#include <filesystem>
#include <string>

#if defined(__APPLE__)
    #include <mach-o/dyld.h>
#endif

namespace Engine {
    namespace fs = std::filesystem;

    std::string GetDataPath(const std::string& subPath) {
        char buffer[4096];
        fs::path exeDir;

#if defined(__APPLE__)
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0) {
            exeDir = fs::path(buffer).parent_path();
        } else {
            exeDir = fs::current_path();
        }
#else
        // Keep original Linux logic for /proc
        try {
            exeDir = fs::canonical("/proc/self/exe").parent_path();
        } catch (...) {
            exeDir = fs::current_path();
        }
#endif

        // Root is the directory containing the 'data' folder
        fs::path root = exeDir.parent_path();
        return (root / "data" / subPath).string();
    }
}
