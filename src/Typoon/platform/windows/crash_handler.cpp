#include "../../low_level/crash_handler.h"

#include <Windows.h>
#include <DbgHelp.h>

#include "log.h"
#include "../../low_level/filesystem.h"


BOOL WINAPI mini_dump_callback(void* currentModule, const PMINIDUMP_CALLBACK_INPUT input, PMINIDUMP_CALLBACK_OUTPUT output)
{
    if (input->CallbackType == IncludeModuleCallback && input->IncludeModule.BaseOfImage != reinterpret_cast<unsigned long long>(currentModule))
    {
        output->ModuleWriteFlags = ModuleWriteModule | ModuleWriteCvRecord | ModuleWriteTlsData;
    }

    return true;
}


long WINAPI on_exception(EXCEPTION_POINTERS* exceptions)
{
    constexpr int flags =
        MiniDumpNormal |
        MiniDumpWithDataSegs |
#ifdef _DEBUG
        MiniDumpWithFullMemory |
#endif
        MiniDumpWithHandleData |
        MiniDumpWithUnloadedModules |
        MiniDumpWithIndirectlyReferencedMemory |
        MiniDumpWithProcessThreadData |
        MiniDumpWithCodeSegs;

    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo{ GetCurrentThreadId(), exceptions, false };
    MINIDUMP_CALLBACK_INFORMATION callbackInfo{ mini_dump_callback, GetModuleHandle(nullptr) };
    const std::filesystem::path& path = get_crash_file_path();
    const HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    logger.Log(reinterpret_cast<unsigned long long>(GetModuleHandle(nullptr)));
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, static_cast<MINIDUMP_TYPE>(flags), &exceptionInfo, nullptr, &callbackInfo);

    return EXCEPTION_CONTINUE_SEARCH;
}


void initialize_crash_handler()
{
    SetUnhandledExceptionFilter(on_exception);
    unsigned long minimumStackSize = 1024 * 17;
    SetThreadStackGuarantee(&minimumStackSize);
}
