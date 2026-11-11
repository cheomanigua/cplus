#pragma once

#include <cstddef>
#include <string>

namespace navalsim::data {

enum class DataErrorCode {
    FileOpenFailed,
    FileReadFailed,
    JsonParseFailed
};

struct DataError {
    DataErrorCode code;
    std::string source;
    std::string message;
    std::size_t bytePosition = 0;
    bool hasBytePosition = false;
};

} //namespace
