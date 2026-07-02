#pragma once
#include <string>

namespace Engine {
    // Returns the absolute path to the data directory
    std::string GetDataPath(const std::string& subPath);
}
