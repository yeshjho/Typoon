#include "../../low_level/file_change_watcher.h"

#include <Windows.h>

#include "../../utils/logger.h"


std::jthread file_change_watcher_thread;

void start_file_change_watcher(std::filesystem::path path, std::function<void()> onChanged)
{
    if (file_change_watcher_thread.joinable()) [[unlikely]]
    {
        logger.Log("File change watcher is already running.", ELogLevel::WARNING);
        return;
    }

    file_change_watcher_thread = std::jthread{ [path = std::move(path), onChanged = std::move(onChanged)](const std::stop_token& stopToken)
    {
        const HANDLE dir = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (dir == INVALID_HANDLE_VALUE) [[unlikely]]
        {
            logger.Log(ELogLevel::ERROR, "Failed to watch path:", path.string(), std::system_category().message(static_cast<int>(GetLastError())));
            return -1;
        }

        unsigned char buffer[1024] = { 0, };
        DWORD bytesReturned;
        while (true)
        {
            if (stopToken.stop_requested()) [[unlikely]]
            {
                break;
            }

            // TODO: This blocks, so stopToken is meaningless. Should use an async method.
            if (!ReadDirectoryChangesW(dir, buffer, sizeof(buffer), true, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION, 
                                       &bytesReturned, nullptr, nullptr)) [[unlikely]]
            {
                logger.Log(ELogLevel::ERROR, "File change watcher error:", std::system_category().message(static_cast<int>(GetLastError())));
                continue;
            }
            
            if (bytesReturned > 0) [[likely]]
            {
                onChanged();
            }
        }

        return 0;
    } };
}

void end_file_change_watcher()
{
    file_change_watcher_thread.request_stop();
    if (file_change_watcher_thread.joinable())
    {
        file_change_watcher_thread.join();
    }
}
