#include "../../low_level/command.h"

#include <Windows.h>

#include "../../utils/logger.h"


std::pair<std::wstring, int> run_command_and_get_output(std::wstring_view command)
{
    std::wstring commandToUse = { command.data(), command.size() };
    commandToUse.append(L"\0");  // Should be null terminated.

    std::wstring output;

    constexpr int bufferSize = 1024;
    char buffer[bufferSize] { 0, };
    wchar_t wideBuffer[bufferSize] { 0, };
    FILE* pipe = _wpopen(commandToUse.data(), L"rt");

    if (!pipe)
    {
        logger.Log(ELogLevel::ERROR, L"Failed to run command", command);
        return { L"", -1 };
    }

    // fgetws is broken. It just uses the lower half of the wchar_t, the contents are the exact same as fgets.
    while (fgets(buffer, bufferSize, pipe))
    {
        MultiByteToWideChar(GetACP(), 0, buffer, -1, wideBuffer, bufferSize);
        output += wideBuffer;
        std::fill_n(buffer, bufferSize, 0);
        std::fill_n(wideBuffer, bufferSize, 0);
    }
    
    const int endOfFileVal = feof(pipe);
    const int closeReturnVal = _pclose(pipe);

    if (!endOfFileVal)
    {
        logger.Log(ELogLevel::WARNING, L"Command output was truncated", command);
    }

    if (!output.empty() && output.back() == L'\n')
    {
        output.pop_back();
    }
    return { std::move(output), closeReturnVal };
}
