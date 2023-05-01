#include "logger.h"

#include <chrono>
#include <ostream>
#include <fstream>


Logger::Logger(LogLevel minLogLevel)
    : mMinLogLevel(minLogLevel)
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

            for (std::wostream* stream : mStreams)
            {
                *stream << line << std::endl;
            }
        }
    })
{
}

Logger::~Logger()
{
    mWriterThread.request_stop();

    std::scoped_lock queueLock{ mLogQueueMutex };
    while (!mLogQueue.empty())
    {
        for (std::wostream* stream : mStreams)
        {
            *stream << mLogQueue.front() << std::endl;
        }
        mLogQueue.pop();
    }

    for (const auto pair : std::views::zip(mStreams, mIsStreamOwned))
    {
        const auto [stream, isOwned] = pair;
        stream->flush();
        if (auto* fstream = dynamic_cast<std::wofstream*>(stream))
        {
            fstream->close();
        }
        if (isOwned)
        {
            delete stream;
        }
    }
}


void Logger::AddOutput(std::wostream& stream)
{
    mStreams.emplace_back(&stream);
    mStreams.back()->imbue(std::locale{ "" });
    mIsStreamOwned.push_back(false);
}


void Logger::AddOutput(const std::filesystem::path& filePath)
{
    mStreams.emplace_back(new std::wofstream{ filePath, std::ios::app });
    mStreams.back()->imbue(std::locale{ "" });
    mIsStreamOwned.push_back(true);
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

