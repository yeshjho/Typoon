#include "logger.h"

#include <chrono>
#include <ostream>
#include <fstream>


Logger::Logger(std::ostream& stream, ELogLevel minLogLevel)
    : mMinLogLevel(minLogLevel), mStream(stream),
    mWriterThread([this](const std::stop_token& stopToken)
    {
        while (true)
        {
            if (stopToken.stop_requested()) [[unlikely]]
            {
                break;
            }

            std::string line;
            {
                std::scoped_lock lock{ mLogQueueMutex };
                if (mLogQueue.empty())
                {
                    std::this_thread::yield();
                    continue;
                }
                line = std::move(mLogQueue.front());
                mLogQueue.pop();
            }

            std::scoped_lock lock{ mStreamMutex };
            mStream << line << std::endl;
        }
    })
{}

Logger::~Logger()
{
    mWriterThread.request_stop();

    std::scoped_lock queueLock{ mLogQueueMutex };
    std::scoped_lock streamLock{ mStreamMutex };
    while (!mLogQueue.empty())
    {
        mStream << mLogQueue.front() << std::endl;
        mLogQueue.pop();
    }

    if (auto& fstream = dynamic_cast<std::ofstream&>(mStream))
    {
        fstream.close();
    }
}


void Logger::Log(const std::string& string, ELogLevel logLevel)
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

    static const std::string logLevelStrings[] = { "", "[DEBUG] ", "[INFO] ", "[WARNING] ", "[ERROR] ", "[FATAL] " };
    static const auto tz = current_zone();

    const auto now = zoned_time{ tz, system_clock::now() };
    std::string line = std::format("{0:%Y-%m-%d %H:%M:%S} ", now) + logLevelStrings[static_cast<int>(logLevel)] + string;

    std::scoped_lock lock{ mLogQueueMutex };
    mLogQueue.push(std::move(line));
}

