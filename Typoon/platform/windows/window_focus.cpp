#include "../../low_level/window_focus.h"

#include <unordered_map>

#include <atlbase.h>
#include <ShlObj.h>
#include <Propkey.h>

#include "../../match/trigger_trees_per_program.h"
#include "log.h"


std::optional<std::wstring> get_program_name(const std::any& data)
{
    DWORD processId;
    if (!data.has_value())
    {
        GUITHREADINFO gti{ .cbSize = sizeof(GUITHREADINFO) };
        if (!GetGUIThreadInfo(0, &gti))
        {
            log_last_error(L"GetGUIThreadInfo failed:");
            return std::nullopt;
        }
        const HWND foregroundWindow = gti.hwndFocus;

        const DWORD threadId = GetWindowThreadProcessId(foregroundWindow, &processId);
        if (!threadId)
        {
            log_last_error(L"GetWindowThreadProcessId failed:");
            return std::nullopt;
        }
    }
    else
    {
        processId = std::any_cast<DWORD>(data);
    }

    const HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, processId);
    if (!process)
    {
        log_last_error(L"OpenProcess failed:");
        return std::nullopt;
    }
    static wchar_t processName[MAX_PATH];
    DWORD processNameLength = MAX_PATH;
    if (!QueryFullProcessImageName(process, 0, processName, &processNameLength))
    {
        log_last_error(L"QueryFullProcessImageName failed:");
        return std::nullopt;
    }
    processName[processNameLength] = L'\0';

    static std::unordered_map<std::wstring, std::wstring> processNameToDescription;
    auto it = processNameToDescription.find(processName);
    if (it == processNameToDescription.end())
    {
        CComPtr<IShellItem2> pItem;
        if (SHCreateItemFromParsingName(processName, nullptr, IID_PPV_ARGS(&pItem)) != 0)
        {
            log_last_error(L"SHCreateItemFromParsingName failed:");
            return std::nullopt;
        }
        CComHeapPtr<WCHAR> pValue;
        if (pItem->GetString(PKEY_FileDescription, &pValue) != 0)
        {
            logger.Log(ELogLevel::INFO, processName, "GetString failed:", get_last_error_string());
            // Some programs don't have a description, so just use the process name.
            it = processNameToDescription.emplace(processName, std::filesystem::path{ processName }.stem()).first;
        }
        else
        {
            it = processNameToDescription.emplace(processName, pValue).first;
        }
    }

    return it->second;
}


void check_for_window_focus_change(const std::any& data)
{
    const DWORD processId = std::any_cast<DWORD>(data);
    const std::optional<std::wstring>& nameOpt = get_program_name(processId);
    if (!nameOpt)
    {
        return;
    }

    const std::wstring& name = nameOpt.value();

    static std::wstring lastName;
    if (lastName == name)
    {
        return;
    }

    set_current_program(name);
    lastName = name;
}
