#pragma once
#include <filesystem>
#include <thread>

#include <moodycamel/blockingconcurrentqueue.h>

#include "util/ToWString.h"


namespace typoon::util
{
    enum class ELogLevel : std::uint8_t
    {
        VERBOSE,
        INFO,
        WARNING,
        ERROR,
        FATAL,
    };


    class Logger
    {
    public:
        static constexpr std::wstring_view LOG_LEVEL_STRINGS[] = { L"VERBOSE", L"   INFO", L"WARNING", L"  ERROR", L"  FATAL", L"  DEBUG" };

    private:
        static constexpr auto LOG_LEVEL_DEBUG = static_cast<ELogLevel>(std::to_underlying(ELogLevel::FATAL) + 1);
        // 로그 레벨은 아니지만 로거 쓰레드 종료 신호용 값으로 사용
        static constexpr auto TERMINATE_SIGNAL = static_cast<ELogLevel>(std::to_underlying(ELogLevel::FATAL) + 2);

        struct LogEntry
        {
            std::chrono::system_clock::time_point timestamp;
            ELogLevel logLevel;
            std::wstring str;
        };

        struct OutputStream
        {
            std::wostream* stream;
            bool isOwned;
            bool autoFlush;
        };

        using TLogEntryQueue = moodycamel::BlockingConcurrentQueue<LogEntry>;

    public:
        explicit Logger(ELogLevel minLogLevel);
        ~Logger();

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;

    public:
        /**
         * @brief 최소 로그 레벨 설정
         */
        void SetMinLogLevel(ELogLevel minLogLevel);

        /**
         * @brief 로그 출력 대상 추가
         * @param ostream 대상 스트림. 로거가 소유권을 가져가지 않으므로 로거보다 오래 살아야 함
         * @param autoFlush 출력 후 플러시 여부
         */
        void AddOutput(std::wostream& ostream, bool autoFlush);
        /**
         * @brief 로그 출력 대상 파일 추가
         * @param filePath 대상 파일 경로
         * @param autoFlush 출력 후 플러시 여부
         */
        void AddOutput(const std::filesystem::path& filePath, bool autoFlush);

        template<CanBeString ...T>
        void Log(const ELogLevel logLevel, T&&... ts) { log(logLevel, concatString(std::forward<T>(ts)...)); }

        template<CanBeString ...T>
        void LogVerbose(T&&... ts) { Log(ELogLevel::VERBOSE, std::forward<T>(ts)...); }
        template<CanBeString ...T>
        void LogInfo(T&&... ts) { Log(ELogLevel::INFO, std::forward<T>(ts)...); }
        template<CanBeString ...T>
        void LogWarning(T&&... ts) { Log(ELogLevel::WARNING, std::forward<T>(ts)...); }
        template<CanBeString ...T>
        void LogError(T&&... ts) { Log(ELogLevel::ERROR, std::forward<T>(ts)...); }
        template<CanBeString ...T>
        void LogFatal(T&&... ts) { Log(ELogLevel::FATAL, std::forward<T>(ts)...); }

        /**
         * @brief 디버그 빌드에서만 동작하는 로그 출력 함수. minLogLevel 무시.
         */
    #ifdef _DEBUG
        template<CanBeString ...T>
        void LogDebug(T&&... ts) { Log(LOG_LEVEL_DEBUG, std::forward<T>(ts)...); }
    #else
        template<CanBeString ...T>
        void LogDebug([[maybe_unused]] T&&... ts) {}
    #endif

    private:
        template<CanBeString ...T>
        std::wstring concatString(T&&... ts);

        void addOutput(OutputStream output);

        void log(ELogLevel logLevel, std::wstring&& str);

        void loggerMain();
        void doLog(const LogEntry& logEntry);


    private:
        ELogLevel mMinLogLevel;
        std::vector<OutputStream> mOutputStreams;

        TLogEntryQueue mPendingLogEntries;
        TLogEntryQueue::consumer_token_t mLogEntryConsumerToken;
        std::thread mThread;
    };


    template <CanBeString ... T>
    std::wstring Logger::concatString(T&&... ts)
    {
        return (to_wstring(std::forward<T>(ts)) + ...);
    }
}
