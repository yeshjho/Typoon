#include "config.h"

#include <json5/json5_input.hpp>
#include <json5/json5_output.hpp>
#include <json5/json5_reflect.hpp>

#include "../low_level/filesystem.h"
#include "../low_level/tray_icon.h"
#include "../parse/parse_keys.h"
#include "json5_util.h"
#include "logger.h"
#include "./string.h"


struct ProgramOverrideForParse
{
    std::vector<std::string> programs;
    bool disable = false;
    std::filesystem::path match_file_path;
    std::vector<std::filesystem::path> includes;
    std::vector<std::filesystem::path> excludes;

    operator ProgramOverride() &&
    {
        makePathAbsolute(match_file_path);
        for (std::filesystem::path& include : includes)
        {
            makePathAbsolute(include);
        }
        for (std::filesystem::path& exclude : excludes)
        {
            makePathAbsolute(exclude);
        }
        ProgramOverride programOverride{
            {},
            disable,
            std::move(match_file_path),
            std::move(includes),
            std::move(excludes)
        };
        programOverride.programs.reserve(programs.size());
        std::ranges::transform(programs, std::back_inserter(programOverride.programs),
            [](const std::string& program)
            {
                return to_u16_string(program);
            });
        return programOverride;
    }

private:
    static void makePathAbsolute(std::filesystem::path& path)
    {
        if (!path.empty() && path.is_relative())
        {
            path = (get_app_data_path() / path).lexically_normal();
        }
    }
};


JSON5_CLASS(ProgramOverrideForParse, programs, disable, match_file_path, includes, excludes)


struct RegexPresetForParse
{
    std::string name;
    std::string expand;

    operator RegexPreset() &&
    {
        return { to_u16_string(name), to_u16_string(expand) };
    }
};


JSON5_CLASS(RegexPresetForParse, name, expand)


struct ConfigForParse
{
    std::filesystem::path match_file_path = "match/matches.json5";
    int max_backspace_count = 5;
    std::string cursor_placeholder = "|_|";

    std::string regex_composite_start = "{{";
    std::string regex_composite_end = "}}";
    std::string regex_preset_surround = "`";
    std::vector<RegexPresetForParse> regex_presets{
        RegexPresetForParse{ to_u8_string(L"초"), to_u8_string(L"[ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅ-ㅎ]") },
        RegexPresetForParse{ to_u8_string(L"중"), to_u8_string(L"[ㅏ-ㅗㅛㅜㅠㅡㅣ]|ㅗㅏ|ㅗㅐ|ㅗㅣ|ㅜㅓ|ㅜㅔ|ㅜㅣ|ㅡㅣ") },
        RegexPresetForParse{ to_u8_string(L"종"), to_u8_string(L"[ㄱㄲㄴㄷㄹㅁㅂㅅ-ㅈㅊ-ㅎ]|ㄱㅅ|ㄴㅈ|ㄴㅎ|ㄹㄱ|ㄹㅁ|ㄹㅂ|ㄹㅅ|ㄹㅌ|ㄹㅍ|ㄹㅎ|ㅂㅅ") },
        RegexPresetForParse{ to_u8_string(L"자"), to_u8_string(L"[ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅ-ㅎ]") },
        RegexPresetForParse{ to_u8_string(L"모"), to_u8_string(L"[ㅏ-ㅗㅛㅜㅠㅡㅣ]|ㅗㅏ|ㅗㅐ|ㅗㅣ|ㅜㅓ|ㅜㅔ|ㅜㅣ|ㅡㅣ") },
        RegexPresetForParse{ to_u8_string(L"쌍"), to_u8_string(L"[ㄲㄸㅃㅆㅉ]") },
        RegexPresetForParse{ to_u8_string(L"겹"), to_u8_string(L"[ㄱㅅ|ㄴㅈ|ㄴㅎ|ㄹㄱ|ㄹㅁ|ㄹㅂ|ㄹㅅ|ㄹㅌ|ㄹㅍ|ㄹㅎ|ㅂㅅ]") },
        RegexPresetForParse{ to_u8_string(L"이중"), to_u8_string(L"[ㅗㅏ|ㅗㅐ|ㅗㅣ|ㅜㅓ|ㅜㅔ|ㅜㅣ|ㅡㅣ]") },
    };

    bool notify_config_load = true;
    bool notify_match_load = true;
    bool notify_on_off = false;

    HotKeyForParse hotkey_toggle_on_off = { .ctrl = true, .shift = true, .alt = true, .key = EKey::S };
    HotKeyForParse hotkey_get_program_name = { .ctrl = true, .shift = true, .alt = true, .key = EKey::D };

    std::vector<ProgramOverrideForParse> program_overrides;
    
    operator Config() &&
    {
        Config config{
            std::move(match_file_path),
            max_backspace_count,
            { cursor_placeholder.begin(), cursor_placeholder.end() },
            { regex_composite_start.begin(), regex_composite_start.end() },
            { regex_composite_end.begin(), regex_composite_end.end() },
            { regex_preset_surround.begin(), regex_preset_surround.end() },
            {},
            notify_config_load,
            notify_match_load,
            notify_on_off,
            { hotkey_toggle_on_off.key, get_combined_modifier(hotkey_toggle_on_off) },
            { hotkey_get_program_name.key, get_combined_modifier(hotkey_get_program_name) },
            {}
        };

        std::transform(std::move_iterator{ regex_presets.begin() }, std::move_iterator{ regex_presets.end() }, std::back_inserter(config.regexPresets),
            [](RegexPresetForParse&& regexPresetForParse) -> RegexPreset
            {
                return std::move(regexPresetForParse);
            });

        std::transform(std::move_iterator{ program_overrides.begin() }, std::move_iterator{ program_overrides.end() }, std::back_inserter(config.programOverrides),
            [](ProgramOverrideForParse&& programOverrideForParse) -> ProgramOverride
            {
                return std::move(programOverrideForParse);
            });

        return config;
    }
};


JSON5_CLASS(ConfigForParse, match_file_path, max_backspace_count, cursor_placeholder, 
    regex_composite_start, regex_composite_end, regex_preset_surround, regex_presets,
    notify_config_load, notify_match_load, notify_on_off, 
    hotkey_toggle_on_off, hotkey_get_program_name, program_overrides)

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
            const std::wstring errorString = json5_error_to_string(err);
            logger.Log(ELogLevel::ERROR, "Failed to read the config file:", errorString);
            show_notification(L"Config File Parse Error", errorString);
            return;
        }
    }

    if (configForParse.match_file_path.is_relative())
    {
        configForParse.match_file_path = (get_app_data_path() / configForParse.match_file_path).lexically_normal();
    }

    if (!std::filesystem::exists(configForParse.match_file_path))
    {
        std::filesystem::create_directories(configForParse.match_file_path.parent_path());
        std::wofstream matchFile{ configForParse.match_file_path };
        matchFile << "{\n"
                     "    matches: [\n"
                     "        {\n"
                     "            trigger: ':Typoon:',\n"
                     "            replace: 'Typoon is awesome!',\n"
                     "        },\n"
                     "    ]\n"
                     "}\n";
    }

    config = std::move(configForParse);
}

const Config& get_config()
{
    return config;
}
