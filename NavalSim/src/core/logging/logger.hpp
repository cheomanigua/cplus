#pragma once

#include <iostream>
#include <string_view>

class Logger
{
public:
    void info(std::string_view message) const
    {
        std::cout << "[INFO] " << message << '\n';
    }

    void error(std::string_view message) const
    {
        std::cerr << "[ERROR] " << message << '\n';
    }
};

