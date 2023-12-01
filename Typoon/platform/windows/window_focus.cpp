#include "../../low_level/window_focus.h"

#include <unordered_map>

#include <atlbase.h>
#include <ShlObj.h>
#include <Propkey.h>

#include "../../match/trigger_trees_per_program.h"
#include "log.h"


void check_for_window_focus_change(const std::any& data)
{
    const DWORD processId = std::any_cast<DWORD>(data);

    const HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, processId);
    if (!process)
    {
        log_last_error(L"OpenProcess failed:");
        return;
    }
    static wchar_t processName[MAX_PATH];
    DWORD processNameLength = MAX_PATH;
    if (!QueryFullProcessImageName(process, 0, processName, &processNameLength))
    {
        log_last_error(L"QueryFullProcessImageName failed:");
        return;
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
            return;
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

    static std::wstring lastProcessDescription;
    if (lastProcessDescription == it->second)
    {
        return;
    }

    set_current_program(it->second);
    lastProcessDescription = it->second;
}
