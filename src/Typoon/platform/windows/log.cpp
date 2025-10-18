#include "log.h"

#include <Windows.h>
#undef ERROR


std::wstring get_last_error_string()
{
    const DWORD errorCode = GetLastError();
    LPWSTR errorText = nullptr;
    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        nullptr,
        errorCode,
        0,
        reinterpret_cast<LPWSTR>(&errorText),
        0,
        nullptr
    );
    std::wstring msg = std::format(L"{0} ({1})", errorText, std::to_wstring(errorCode));
    LocalFree(errorText);
    return msg;
}


void log_last_error(const std::wstring& additionalMsg, LogLevel logLevel)
{
    logger.Log(logLevel, additionalMsg, get_last_error_string());
}
