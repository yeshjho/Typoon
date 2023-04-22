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
    else
    {
        std::unreachable();
    }
}
}


struct LogLevel
{
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

    constexpr auto operator<=>(const LogLevel& other) const = default;

    ELogLevel logLevel;
};

namespace ELogLevel
{

constexpr struct LogLevelMin : LogLevel {} MIN{ LogLevel::ELogLevel::MIN };

constexpr struct LogLevelDebug : LogLevel {} DEBUG{ LogLevel::ELogLevel::DEBUG };
constexpr struct LogLevelInfo : LogLevel {} INFO{ LogLevel::ELogLevel::INFO };
constexpr struct LogLevelWarning : LogLevel {} WARNING{ LogLevel::ELogLevel::WARNING };
constexpr struct LogLevelError : LogLevel {} ERROR{ LogLevel::ELogLevel::ERROR };
constexpr struct LogLevelFatal : LogLevel {} FATAL{ LogLevel::ELogLevel::FATAL };

constexpr struct LogLevelMax : LogLevel {} MAX{ LogLevel::ELogLevel::MAX };

}


class Logger
{
public:
    Logger(std::ostream& stream, LogLevel minLogLevel = ELogLevel::ERROR);
    ~Logger();

    Logger(const Logger& other) = delete;
    Logger(Logger&& other) noexcept = delete;
    Logger& operator=(const Logger& other) = delete;
    Logger& operator=(Logger&& other) noexcept = delete;

    void Log(const std::string& string, LogLevel logLevel = ELogLevel::INFO);

    template<_impl::CanBeString T>
    void Log(const T& t, LogLevel logLevel = ELogLevel::INFO);

    template<_impl::CanBeString T, _impl::CanBeString ...Ts>
    void Log(LogLevel logLevel, const T& t, const Ts& ...ts);

#ifndef _DEBUG
    void Log(const std::string&, ELogLevel::LogLevelDebug) {}

    template<_impl::CanBeString T>
    void Log(const T&, ELogLevel::LogLevelDebug) {}

    template<_impl::CanBeString T, _impl::CanBeString ...Ts>
    void Log(ELogLevel::LogLevelDebug, const T&, const Ts& ...) {}
#endif


private:
    LogLevel mMinLogLevel;

    std::ostream& mStream;

    std::jthread mWriterThread;

    std::queue<std::string> mLogQueue;
    std::mutex mLogQueueMutex;
    std::condition_variable mLogQueueConditionVariable;
};


template<_impl::CanBeString T>
void Logger::Log(const T& t, LogLevel logLevel)
{
    if (logLevel < mMinLogLevel)
    {
        return;
    }

    Log(_impl::to_string(t), logLevel);
}

template<_impl::CanBeString T, _impl::CanBeString ...Ts>
void Logger::Log(LogLevel logLevel, const T& t, const Ts& ...ts)
{
    if (logLevel < mMinLogLevel)
    {
        return;
    }

    Log((_impl::to_string(t) + ... + (" " + _impl::to_string(ts))), logLevel);
}


inline Logger g_console_logger{ std::cout, ELogLevel::DEBUG };
