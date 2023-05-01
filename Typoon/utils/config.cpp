#include "config.h"

#include <json5/json5_input.hpp>
#include <json5/json5_output.hpp>
#include <json5/json5_reflect.hpp>

#include "../low_level/filesystem.h"
#include "logger.h"


namespace json5::detail
{

template <>
error read(const value& in, std::filesystem::path& out)
{
    if (in.is_string())
    {
        out = in.get_c_str();
        return { error::none };
    }

    return { error::string_expected };
}

template<>
value write(writer& w, const std::filesystem::path& in)
{
    return write(w, in.generic_string());
}

}


struct ConfigForParse
{
    std::filesystem::path match_file_path = "match/matches.json5";

    operator Config() const &&
    {
        return { match_file_path };
    }
};


JSON5_CLASS(ConfigForParse, match_file_path)

Config config;


// TODO: thread safety

void read_config_file(const std::filesystem::path& filePath)
{
    ConfigForParse configForParse;

    if (!std::filesystem::exists(filePath))
    {
        json5::to_file(filePath.generic_string(), configForParse);
    }
    else
    {
        if (const json5::error err = json5::from_file(filePath.generic_string(), configForParse);
            err != json5::error::none)
        {
            g_console_logger.Log(ELogLevel::FATAL, "Failed to read the config file");
            std::exit(-1);
        }
    }

    if (configForParse.match_file_path.is_relative())
    {
        configForParse.match_file_path = get_app_data_path() / configForParse.match_file_path;
    }

    if (!std::filesystem::exists(configForParse.match_file_path))
    {
        std::filesystem::create_directories(configForParse.match_file_path.parent_path());
        std::wofstream matchFile{ configForParse.match_file_path };
        matchFile << "{}";
    }

    config = std::move(configForParse);
}

const Config& get_config()
{
    return config;
}
