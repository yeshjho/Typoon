#include "logger.h"

#include <chrono>
#include <ostream>
#include <fstream>


Logger::Logger(std::wostream& stream, LogLevel minLogLevel)
    : mMinLogLevel(minLogLevel)
    , mStream(stream)
    , mWriterThread([this](const std::stop_token& stopToken)
    {
        while (true)
        {
            std::unique_lock queueLock{ mLogQueueMutex };
            mLogQueueConditionVariable.wait(queueLock, [this] { return !mLogQueue.empty(); });

            if (stopToken.stop_requested()) [[unlikely]]
            {
                queueLock.unlock();
                break;
            }

            std::wstring line;
            line = std::move(mLogQueue.front());
            mLogQueue.pop();

            queueLock.unlock();
            
            mStream << line << std::endl;
        }
    })
{
    mStream.imbue(std::locale{ "" });
}

Logger::~Logger()
{
    mWriterThread.request_stop();

    std::scoped_lock queueLock{ mLogQueueMutex };
    while (!mLogQueue.empty())
    {
        mStream << mLogQueue.front() << std::endl;
        mLogQueue.pop();
    }

    if (auto& fstream = dynamic_cast<std::wofstream&>(mStream))
    {
        fstream.close();
    }
}


void Logger::Log(const std::wstring& string, LogLevel logLevel)
{
    using namespace std::chrono;

    if (logLevel < mMinLogLevel)
    {
        return;
    }

    if (logLevel <= ELogLevel::MIN || logLevel >= ELogLevel::MAX) [[unlikely]]
    {
        return;
    }

    static const std::wstring logLevelStrings[] = { L"", L"[DEBUG] ", L"[INFO] ", L"[WARNING] ", L"[ERROR] ", L"[FATAL] " };
    static const auto tz = current_zone();

    const auto now = zoned_time{ tz, system_clock::now() };
    std::wstring line = std::format(L"{0:%Y-%m-%d %H:%M:%S} ", now) + logLevelStrings[static_cast<int>(logLevel.logLevel)] + string;

    {
        std::scoped_lock lock{ mLogQueueMutex };
        mLogQueue.push(std::move(line));
    }
    mLogQueueConditionVariable.notify_one();
}

