#include <doctest.h>

#include <array>
#include <chrono>
#include <fstream>
#include <ranges>
#include <sstream>
#include <thread>

#include "Typoon/logger/Logger.h"

#include "util/DocTestWStringSupport.h"


using typoon::util::ELogLevel;
using typoon::util::Logger;

namespace
{
    void check_log(const std::wstring& log, const ELogLevel expectedLogLevel, const std::wstring_view expected)
    {
        std::wistringstream ss{ log };
        ss.ignore(std::numeric_limits<std::streamsize>::max(), L']');  // 타임스탬프 무시
        ss.ignore(2);  // "] "

        std::wstring logLevel;
        std::getline(ss, logLevel, L']');
        REQUIRE(logLevel == Logger::LOG_LEVEL_STRINGS[std::to_underlying(expectedLogLevel)]);

        ss.ignore(1);  // " "

        const std::streampos pos = ss.tellg();
        const std::wstring_view content = std::wstring_view{ log }.substr(pos, log.size() - static_cast<size_t>(pos) - 1);  // 마지막 \n 무시
        CHECK(content == expected);
    }
}


TEST_SUITE("Logger")
{
    TEST_CASE("기본 기능")
    {
        SUBCASE("기본 로깅")
        {
            constexpr std::wstring_view toLog = L"테스트 로그 메시지";

            std::wstringstream ss;
            {
                Logger logger{ ELogLevel::VERBOSE };
                logger.AddOutput(ss, false);

                logger.LogInfo(toLog);
            }

            const std::wstring logOutput = ss.str();
            check_log(logOutput, ELogLevel::INFO, toLog);
        }

        SUBCASE("파일 로깅")
        {
            const std::filesystem::path filePath = "temp.txt";
            constexpr std::wstring_view expected = L"Test log message";

            {
                Logger logger{ ELogLevel::VERBOSE };
                logger.AddOutput(filePath, false);

                logger.LogWarning(expected);
            }

            {
                std::wifstream file{ filePath };
                std::wstring logOutput;
                std::getline(file, logOutput);
                logOutput.push_back(L'\n');
                check_log(logOutput, ELogLevel::WARNING, expected);
            }
            std::filesystem::remove(filePath);
        }

        SUBCASE("로그 레벨 필터링")
        {
            constexpr std::wstring_view verboseLog = L"Verbose";
            constexpr std::wstring_view infoLog = L"Info";
            constexpr std::wstring_view warningLog = L"Warning";
            constexpr std::wstring_view errorLog = L"ErrorLog";
            // FATAL은 breakpoint 걸리므로 제외
            constexpr std::wstring_view debugLog = L"DebugLog";

            static constexpr auto LOG_LEVEL_DEBUG = static_cast<ELogLevel>(std::to_underlying(ELogLevel::FATAL) + 1);
#ifdef _DEBUG
            constexpr std::pair<ELogLevel, std::wstring_view> expected[] = {
                { ELogLevel::VERBOSE, verboseLog },
                { ELogLevel::INFO, infoLog },
                { ELogLevel::WARNING, warningLog },
                { ELogLevel::ERROR, errorLog },
                { LOG_LEVEL_DEBUG, debugLog },

                { ELogLevel::INFO, infoLog },
                { ELogLevel::WARNING, warningLog },
                { ELogLevel::ERROR, errorLog },
                { LOG_LEVEL_DEBUG, debugLog },

                { ELogLevel::WARNING, warningLog },
                { ELogLevel::ERROR, errorLog },
                { LOG_LEVEL_DEBUG, debugLog },

                { ELogLevel::ERROR, errorLog },
                { LOG_LEVEL_DEBUG, debugLog },
            };
#else
            constexpr std::pair<ELogLevel, std::wstring_view> expected[] = {
                { ELogLevel::VERBOSE, verboseLog },
                { ELogLevel::INFO, infoLog },
                { ELogLevel::WARNING, warningLog },
                { ELogLevel::ERROR, errorLog },

                { ELogLevel::INFO, infoLog },
                { ELogLevel::WARNING, warningLog },
                { ELogLevel::ERROR, errorLog },

                { ELogLevel::WARNING, warningLog },
                { ELogLevel::ERROR, errorLog },

                { ELogLevel::ERROR, errorLog },
            };
#endif
            constexpr size_t expectedSize = std::size(expected);

            const auto lambdaLogAllLevels = [=](Logger& logger)
            {
                logger.LogVerbose(verboseLog);
                logger.LogInfo(infoLog);
                logger.LogWarning(warningLog);
                logger.LogError(errorLog);
                logger.LogDebug(debugLog);
            };

            std::wstringstream ss;
            {
                Logger logger{ ELogLevel::VERBOSE };
                logger.AddOutput(ss, false);
                lambdaLogAllLevels(logger);

                logger.SetMinLogLevel(ELogLevel::INFO);
                lambdaLogAllLevels(logger);

                logger.SetMinLogLevel(ELogLevel::WARNING);
                lambdaLogAllLevels(logger);

                logger.SetMinLogLevel(ELogLevel::ERROR);
                lambdaLogAllLevels(logger);
            }

            std::wstring line;
            size_t index = 0;
            while (std::getline(ss, line))
            {
                REQUIRE(index < expectedSize);
                line.push_back(L'\n');
                check_log(line, expected[index].first, expected[index].second);
                index++;
            }
        }

        SUBCASE("각종 타입")
        {
            std::wstringstream ss;
            {
                Logger logger{ ELogLevel::VERBOSE };
                logger.AddOutput(ss, false);

                logger.LogError("test", L"테스트", 'c', L'가', 1);
            }
            const std::wstring logOutput = ss.str();
            check_log(logOutput, ELogLevel::ERROR, L"test테스트c가1");
        }
    }

    TEST_CASE("thread safety")
    {
        constexpr int threadCount = 32;
        constexpr int logCount = 1'000;

        std::wstringstream ss;
        {
            Logger logger{ ELogLevel::VERBOSE };
            logger.AddOutput(ss, false);

            std::array<std::thread, threadCount> threads;
            for (int i = 0; i < threadCount; i++)
            {
                threads[i] = std::thread{
                    [&logger, i]()
                    {
                        for (int j = 0; j < logCount; j++)
                        {
                            logger.LogInfo('|', i, ' ', j);
                        }
                    }
                };
            }
            for (std::thread& thread : threads)
            {
                thread.join();
            }
        }

        std::array<std::array<bool, logCount>, threadCount> logTracker{};
        std::wstring line;
        while (std::getline(ss, line))
        {
            const std::wstring_view lineSv{ line };
            const size_t contentPos = lineSv.find(L'|');
            REQUIRE(contentPos != std::wstring::npos);

            const std::wstring_view threadIndexAndLogIndex = lineSv.substr(contentPos + 1);
            const size_t dividerPos = threadIndexAndLogIndex.find(L' ');
            REQUIRE(dividerPos != std::wstring::npos);

            int threadIndex = 0;
            int logIndex = 0;
            REQUIRE_NOTHROW(threadIndex = std::stoi(std::wstring{ threadIndexAndLogIndex.substr(0, dividerPos) }));
            REQUIRE_NOTHROW(logIndex = std::stoi(std::wstring{ threadIndexAndLogIndex.substr(dividerPos + 1) }));
            logTracker[threadIndex][logIndex] = true;
        }

        const bool hasAllLogs = std::ranges::all_of(logTracker,
            [](const std::array<bool, logCount>& thread)
            {
                return std::ranges::all_of(thread, [](const bool b) { return b; });
            });
        CHECK(hasAllLogs);
    }
}
