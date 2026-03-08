#include "logger.hpp"
#include <fstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

namespace logger
{
    static std::ofstream log_file;
    static std::mutex log_mutex;

#ifdef _WIN32
    static bool is_valid_utf8(const std::string &input)
    {
        int expected = 0;
        for (unsigned char c : input)
        {
            if (expected == 0)
            {
                if ((c >> 5) == 0x6)
                    expected = 1;
                else if ((c >> 4) == 0xE)
                    expected = 2;
                else if ((c >> 3) == 0x1E)
                    expected = 3;
                else if ((c >> 7) != 0)
                    return false;
            }
            else
            {
                if ((c >> 6) != 0x2)
                    return false;
                --expected;
            }
        }
        return expected == 0;
    }

    static std::string ansi_to_utf8(const std::string &ansi)
    {
        if (ansi.empty())
            return {};

        int wide_len = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, nullptr, 0);
        if (wide_len <= 0)
            return ansi;

        std::wstring wide(static_cast<size_t>(wide_len), L'\0');
        MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, &wide[0], wide_len);

        int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8_len <= 0)
            return ansi;

        std::string utf8(static_cast<size_t>(utf8_len), '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &utf8[0], utf8_len, nullptr, nullptr);

        if (!utf8.empty() && utf8.back() == '\0')
            utf8.pop_back();
        return utf8;
    }

    static std::string normalize_to_utf8(const std::string &text)
    {
        if (text.empty())
            return {};
        if (is_valid_utf8(text))
            return text;
        return ansi_to_utf8(text);
    }
#endif

    void init_logger()
    {
        try
        {
            if (!std::filesystem::exists("logs"))
            {
                std::filesystem::create_directory("logs");
            }
            const auto log_path = std::filesystem::path("logs/app.log");
            const bool need_bom = !std::filesystem::exists(log_path) ||
                                  std::filesystem::file_size(log_path) == 0;

            log_file.open(log_path, std::ios_base::app | std::ios_base::binary);
            if (!log_file.is_open())
            {
                std::cerr << "Failed to open log file." << std::endl;
            }
            else if (need_bom)
            {
                const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
                log_file.write(reinterpret_cast<const char *>(bom), sizeof(bom));
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
#ifdef _WIN32
    const std::string level_text = normalize_to_utf8(level);
    const std::string message_text = normalize_to_utf8(message);
#else
    const std::string &level_text = level;
    const std::string &message_text = message;
#endif

        log_file << "[" << std::put_time(&buf, "%Y-%m-%d %X") << "] "
         << "[" << level_text << "] "
         << message_text << std::endl;
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