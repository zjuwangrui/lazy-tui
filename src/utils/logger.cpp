#include "logger.hpp"
#include <fstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <filesystem>

namespace logger
{
    static std::ofstream log_file;
    static std::mutex log_mutex;

    void init_logger()
    {
        try
        {
            if (!std::filesystem::exists("logs"))
            {
                std::filesystem::create_directory("logs");
            }
            log_file.open("logs/app.log", std::ios_base::app);
            if (!log_file.is_open())
            {
                std::cerr << "Failed to open log file." << std::endl;
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }

    static void log(const std::string &level, const std::string &message)
    {
        if (!log_file.is_open())
            return;

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        
        std::tm buf;
#ifdef _WIN32
        localtime_s(&buf, &in_time_t);
#else
        localtime_r(&in_time_t, &buf);
#endif

        std::lock_guard<std::mutex> lock(log_mutex);
        log_file << "[" << std::put_time(&buf, "%Y-%m-%d %X") << "] "
                 << "[" << level << "] "
                 << message << std::endl;
    }

    void log_info(const std::string &message)
    {
        log("INFO", message);
    }

    void log_error(const std::string &message)
    {
        log("ERROR", message);
    }
}