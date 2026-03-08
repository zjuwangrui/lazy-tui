#pragma once
#include <string>

namespace logger
{
    void init_logger();
    void log_info(const std::string &message);
    void log_error(const std::string &message);
}