#include "../../low_level/filesystem.h"

#include <ShlObj_core.h>

#include "../../utils/logger.h"


const std::filesystem::path& get_app_data_path()
{
    static std::filesystem::path appDataPath{
        []()
        {
            PWSTR path = nullptr;
            if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &path)))
            {
                logger.Log(ELogLevel::FATAL, "SHGetKnownFolderPath failed:", std::system_category().message(static_cast<int>(GetLastError())));
                std::exit(-1);
            }

            auto appPath = std::filesystem::path{ path } / "Typoon";
            std::filesystem::create_directories(appPath);

            CoTaskMemFree(path);

            return appPath;
        }()
    };

    return appDataPath;
}


const std::filesystem::path& get_config_file_path()
{
    static std::filesystem::path configPath = get_app_data_path() / "config.json5";
    return configPath;
}


std::filesystem::path get_log_file_path()
{
    using namespace std::chrono;

    static const auto tz = current_zone();

    const auto now = zoned_time{ tz, system_clock::now() };
    std::filesystem::path path = get_app_data_path() / "logs" / std::format(L"{0:%Y-%m-%d}.txt", now);
    std::filesystem::create_directories(path.parent_path());
    return path;
}