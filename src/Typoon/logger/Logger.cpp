#include "Logger.h"

#include <chrono>
#include <fstream>

#include "util/Debugging.h"


namespace typoon::util
{
    Logger::Logger(const ELogLevel minLogLevel)
        : mMinLogLevel(minLogLevel)
        , mLogEntryConsumerToken(mPendingLogEntries)
        , mThread(std::thread{ [this]() { loggerMain(); } })
    {}

    Logger::~Logger()
    {
        mPendingLogEntries.enqueue({ .timestamp = {}, .logLevel = TERMINATE_SIGNAL, .str = {} });
        mThread.join();

        for (const auto& [stream, isOwned, _] : mOutputStreams)
        {
            stream->flush();
            if (isOwned)
            {
                delete stream;
            }
        }
    }

    void Logger::SetMinLogLevel(const ELogLevel minLogLevel)
    {
        mMinLogLevel = minLogLevel;
    }

    void Logger::AddOutput(std::wostream& ostream, const bool autoFlush)
    {
        addOutput({ .stream = &ostream, .isOwned = false, .autoFlush = autoFlush });
    }

    void Logger::AddOutput(const std::filesystem::path& filePath, const bool autoFlush)
    {
        auto* file = new std::wofstream{ filePath, std::ios::app };
        addOutput({ .stream = file, .isOwned = true, .autoFlush = autoFlush });
    }

    void Logger::addOutput(OutputStream output)
    {
        // 로케일 설정을 해주지 않으면 깨지는 경우가 있음
        output.stream->imbue(std::locale{ "" });
        mOutputStreams.emplace_back(output);
    }

    void Logger::log(const ELogLevel logLevel, std::wstring&& str)
    {
        if (logLevel < mMinLogLevel || str.empty())
        {
            return;
        }

    #ifdef _DEBUG
        if (logLevel == ELogLevel::FATAL)
        {
            BREAKPOINT_IF_DEBUGGING();
        }
    #endif

        mPendingLogEntries.enqueue(LogEntry{
            .timestamp = std::chrono::system_clock::now(),
            .logLevel = logLevel,
            .str = std::move(str),
        });
    }

    void Logger::loggerMain()
    {
        LogEntry entry;
        while (true)
        {
            mPendingLogEntries.wait_dequeue(mLogEntryConsumerToken, entry);
            if (entry.logLevel == TERMINATE_SIGNAL)
            {
                while (mPendingLogEntries.try_dequeue(mLogEntryConsumerToken, entry))
                {
                    doLog(entry);
                }
                break;
            }

            doLog(entry);
        }
    }

    void Logger::doLog(const LogEntry& logEntry)
    {
        using namespace std::chrono;

        static const time_zone* const tz = current_zone();

        const auto time = zoned_time{ tz, logEntry.timestamp };
        const std::wstring line = std::format(L"[{:%Y-%m-%d %H:%M:%S}] [{}] {}\n", time, LOG_LEVEL_STRINGS[std::to_underlying(logEntry.logLevel)], logEntry.str);
        for (auto& [stream, _, autoFlush] : mOutputStreams)
        {
            *stream << line;
            if (autoFlush)
            {
                stream->flush();
            }
        }
    }
}
