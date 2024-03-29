#pragma once
#include <filesystem>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <queue>
#include <ranges>
#undef ERROR
#undef DEBUG


namespace _impl
{
template<typename T>
concept CanConstructWStringWith = requires(T t)
{
    std::wstring{ t };
};

template<typename T>
concept CanConvertibleToWString = requires(T t)
{
    std::to_wstring(t);
};

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
concept CanBeString = CanConstructWStringWith<T> || CanConvertibleToWString<T> || CanConstructStringWith<T> || CanConvertibleToString<T> || std::is_same_v<std::remove_cvref_t<T>, bool>;

template<CanBeString T>
std::wstring to_wstring(const T& t)
{
    if constexpr (std::is_same_v<std::remove_cvref_t<T>, bool>)
    {
        return t ? L"true" : L"false";
    }
    else if constexpr (CanConstructWStringWith<T>)
    {
        return std::wstring{ t };
    }
    else if constexpr (CanConvertibleToWString<T>)
    {
        return std::to_wstring(t);
    }
    else if constexpr (CanConstructStringWith<T>)
    {
        std::string s{ t };
        return std::wstring{ s.begin(), s.end() };
    }
    else if constexpr (CanConvertibleToString<T>)
    {
        std::string s = std::to_string(t);
        return std::wstring{ s.begin(), s.end() };
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
    Logger(LogLevel minLogLevel = ELogLevel::ERROR);
    ~Logger();

    Logger(const Logger& other) = delete;
    Logger(Logger&& other) noexcept = delete;
    Logger& operator=(const Logger& other) = delete;
    Logger& operator=(Logger&& other) noexcept = delete;

    void AddOutput(std::wostream& stream);
    void AddOutput(const std::filesystem::path& filePath);

    void Log(const std::wstring& string, LogLevel logLevel = ELogLevel::INFO);

    template<_impl::CanBeString T>
    void Log(const T& t, LogLevel logLevel = ELogLevel::INFO);

    template<_impl::CanBeString T, _impl::CanBeString ...Ts>
    void Log(LogLevel logLevel, const T& t, const Ts& ...ts);

#ifndef _DEBUG
    void Log(const std::wstring&, ELogLevel::LogLevelDebug) {}

    template<_impl::CanBeString T>
    void Log(const T&, ELogLevel::LogLevelDebug) {}

    template<_impl::CanBeString T, _impl::CanBeString ...Ts>
    void Log(ELogLevel::LogLevelDebug, const T&, const Ts& ...) {}
#endif


private:
    LogLevel mMinLogLevel;

    std::vector<std::wostream*> mStreams;
    std::vector<bool> mIsStreamOwned;

    std::jthread mWriterThread;

    std::queue<std::wstring> mLogQueue;
    std::mutex mLogQueueMutex;
    std::condition_variable mLogQueueConditionVariable;
    std::atomic<bool> mShouldTerminate;
};


template<_impl::CanBeString T>
void Logger::Log(const T& t, LogLevel logLevel)
{
    if (logLevel < mMinLogLevel)
    {
        return;
    }

    Log(_impl::to_wstring(t), logLevel);
}

template<_impl::CanBeString T, _impl::CanBeString ...Ts>
void Logger::Log(LogLevel logLevel, const T& t, const Ts& ...ts)
{
    if (logLevel < mMinLogLevel)
    {
        return;
    }

    Log((_impl::to_wstring(t) + ... + (L" " + _impl::to_wstring(ts))), logLevel);
}

#ifdef _DEBUG
inline Logger logger{ ELogLevel::DEBUG };
#else
inline Logger logger{ ELogLevel::INFO };
#endif
