#include "Core/PathResolver.hpp"
#include <filesystem>

namespace Engine {
    std::string GetDataPath(const std::string& subPath) {
        namespace fs = std::filesystem;
        // Logic to find the root and append subPath
        fs::path exePath = fs::canonical("/proc/self/exe");
        fs::path root = exePath.parent_path().parent_path();
        return (root / "data" / subPath).string();
    }
}
