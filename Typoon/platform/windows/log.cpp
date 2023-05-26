#include "log.h"

#include <Windows.h>
#undef ERROR

#include "../../utils/logger.h"


void log_last_error(const std::wstring& additionalMsg)
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
    const std::wstring msg = additionalMsg + L" " + errorText;
    logger.Log(ELogLevel::ERROR, msg);
    LocalFree(errorText);
}
