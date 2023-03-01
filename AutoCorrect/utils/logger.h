#pragma once
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <queue>
#include <ranges>
#undef ERROR


namespace _impl
{
template<typename T>
concept CanConstructStringWith = requires(T t)
{
    std::string{ t };
};

template<typename T>
concept CanConvertibleToString = requires(T t)
{
    std::to_string(t);
};

template<typename T>
concept CanBeString = CanConstructStringWith<T> || CanConvertibleToString<T>;

template<CanBeString T>
std::string to_string(const T& t)
{
    if constexpr (CanConstructStringWith<T>)
    {
        return std::string{ t };
    }
    else if constexpr (CanConvertibleToString<T>)
    {
        return std::to_string(t);
    }
    std::unreachable();
}
}


enum class ELogLevel
{
    MIN,

    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,

    MAX
};

class Logger
{
public:
    Logger(std::ostream& stream, ELogLevel minLogLevel = ELogLevel::ERROR);
    ~Logger();

    Logger(const Logger& other) = delete;
    Logger(Logger&& other) noexcept = delete;
    Logger& operator=(const Logger& other) = delete;
    Logger& operator=(Logger&& other) noexcept = delete;

    void Log(const std::string& string, ELogLevel logLevel = ELogLevel::INFO);
    template<_impl::CanBeString T>
    void Log(const T& t, ELogLevel logLevel = ELogLevel::INFO);
    template<_impl::CanBeString T, _impl::CanBeString ...Ts>
    void Log(ELogLevel logLevel, const T& t, const Ts& ...ts);

private:
    ELogLevel mMinLogLevel;

    std::ostream& mStream;
    std::mutex mStreamMutex;

    std::jthread mWriterThread;

    std::queue<std::string> mLogQueue;
    std::mutex mLogQueueMutex;
};


template<_impl::CanBeString T>
void Logger::Log(const T& t, ELogLevel logLevel)
{
    if (logLevel < mMinLogLevel)
    {
        return;
    }

    Log(_impl::to_string(t), logLevel);
}

template<_impl::CanBeString T, _impl::CanBeString ...Ts>
void Logger::Log(ELogLevel logLevel, const T& t, const Ts& ...ts)
{
    if (logLevel < mMinLogLevel)
    {
        return;
    }

    Log((_impl::to_string(t) + ... + (" " + _impl::to_string(ts))), logLevel);
}


inline Logger g_console_logger{ std::cout, ELogLevel::DEBUG };
