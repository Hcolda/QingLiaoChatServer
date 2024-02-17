#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <fstream>
#include <atomic>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <functional>

namespace Log
{
    template<typename T>
    concept StreamType = requires (T t) {
        std::cout << t;
    };

    class Logger
    {
    public:
        enum class LogMode
        {
            LogINFO = 0,
            LogWARNING,
            LogERROR,
            LogCRITICAL,
            LogDEBUG
        };

        Logger() :
            m_isRunning(true),
            m_thread(std::bind(&Logger::workFunction, this))
        {
            if (!openFile())
                throw std::runtime_error("Can't no open log file.");
        }

        ~Logger()
        {
            m_isRunning = false;
            m_cv.notify_all();

            if (m_thread.joinable())
                m_thread.join();
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        constexpr void info(Args&&... args)
        {
            print(LogMode::LogINFO, std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        constexpr void warning(Args&&... args)
        {
            print(LogMode::LogWARNING, std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        constexpr void error(Args&&... args)
        {
            print(LogMode::LogERROR, std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        constexpr void critical(Args&&... args)
        {
            print(LogMode::LogCRITICAL, std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        constexpr void debug(Args&&... args)
        {
#ifdef _DEBUG
            print(LogMode::LogDEBUG, std::forward<Args>(args)...);
#endif // _DEBUG
        }

        template<typename... Args>
            requires ((StreamType<Args>) && ...)
        void print(LogMode mode, Args&&... args)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_msgQueue.push(std::bind([mode = std::move(mode), this](auto... args) {
                std::string modeString;

                switch (mode)
                {
                case Logger::LogMode::LogINFO:
                    modeString = "[INFO]";
                    break;
                case Logger::LogMode::LogWARNING:
                    modeString = "[WRANING]";
                    break;
                case Logger::LogMode::LogERROR:
                    modeString = "[ERROR]";
                    break;
                case Logger::LogMode::LogCRITICAL:
                    modeString = "[CRITICAL]";
                    break;
                case Logger::LogMode::LogDEBUG:
                    modeString = "[DEBUG]";
                    break;
                default:
                    break;
                }

                std::cout << generateTimeFormatString() << modeString;
                ((std::cout << args), ...);
                std::cout << '\n';

                m_file << generateTimeFormatString() << modeString;
                ((m_file << std::forward<decltype(args)>(args)), ...);
                m_file << std::endl;
                }, std::forward<Args>(args)...));
            m_cv.notify_all();
        }

    protected:
        static std::string generateFileName()
        {
            auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&t), "%Y-%m-%d.log");
            return ss.str();
        }

        static std::string generateTimeFormatString()
        {
            auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&t), "[%H:%M:%S]");
            return ss.str();
        }

        bool openFile()
        {
            std::filesystem::create_directory("./logs");
            m_file.open("./logs/" + generateFileName(), std::ios_base::app);
            return static_cast<bool>(m_file);
        }

        void workFunction()
        {
            while (m_isRunning)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [&]() {return !m_msgQueue.empty() || !m_isRunning; });
                if (!m_isRunning && m_msgQueue.empty())
                    return;
                else if (m_msgQueue.empty())
                    continue;
                std::function<void()> getFunc = std::move(m_msgQueue.front());
                m_msgQueue.pop();
                getFunc();
            }
        }

    private:
        std::ofstream                       m_file;
        std::condition_variable             m_cv;
        std::mutex                          m_mutex;
        std::atomic<bool>                   m_isRunning;
        std::queue<std::function<void()>>   m_msgQueue;
        std::thread                         m_thread;

    };
}

#endif // !LOGGER_HPP
