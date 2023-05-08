#include "../../low_level/file_change_watcher.h"

#include <Windows.h>

#include "../../utils/logger.h"


std::jthread file_change_watcher_thread;


FileChangeWatcher::FileChangeWatcher(std::function<void()> onChanged)
{
    mEvents.emplace_back(CreateEvent(nullptr, false, false, nullptr));  // "Kill" event
    mThread = std::jthread{
        [this, onChanged = std::move(onChanged)](const std::stop_token& stopToken)
        {
            while (true)
            {
                if (stopToken.stop_requested())
                {
                    return;
                }

                const DWORD result = WaitForMultipleObjectsEx(static_cast<DWORD>(mEvents.size()), mEvents.data(), false, INFINITE, true);
                if (result == WAIT_FAILED)
                {
                    logger.Log(ELogLevel::ERROR, "WaitForMultipleObjectsEx error:", GetLastError(), std::system_category().message(static_cast<int>(GetLastError())));
                    return;
                }

                if (result != WAIT_OBJECT_0)  // not a "kill" signal
                {
                    onChanged();
                    readDirectoryChanges(static_cast<int>(result - WAIT_OBJECT_0 - 1));
                }
            }
        }
    };
}


FileChangeWatcher::~FileChangeWatcher()
{
    mThread.request_stop();
    SetEvent(mOverlappeds.front());
    if (mThread.joinable())
    {
        mThread.join();
    }

    for (const HANDLE file : mDirectories)
    {
        CloseHandle(file);
    }
    for (const HANDLE handle : mEvents)
    {
        CloseHandle(handle);
    }
    for (const void* overlappedPtr : mOverlappeds)
    {
        const auto* overlapped = static_cast<const OVERLAPPED*>(overlappedPtr);
        delete overlapped;
    }
}


void FileChangeWatcher::AddWatchingDirectory(const std::filesystem::path& path)
{
    const HANDLE dir = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
    if (dir == INVALID_HANDLE_VALUE) [[unlikely]]
    {
        logger.Log(ELogLevel::ERROR, "CreateFile error:", path.string(), std::system_category().message(static_cast<int>(GetLastError())));
        return;
    }
    mDirectories.emplace_back(dir);

    const auto overlapped = new OVERLAPPED{};
    overlapped->hEvent = CreateEvent(nullptr, false, false, nullptr);
    mEvents.emplace_back(overlapped->hEvent);
    mOverlappeds.emplace_back(overlapped);

    readDirectoryChanges(static_cast<int>(mDirectories.size() - 1));
}


void FileChangeWatcher::readDirectoryChanges(int index) const
{
    if (!ReadDirectoryChangesW(mDirectories.at(index), nullptr, 0, false, FILE_NOTIFY_CHANGE_LAST_WRITE, nullptr,
        static_cast<OVERLAPPED*>(mOverlappeds.at(index)), nullptr))
    {
        logger.Log(ELogLevel::ERROR, "ReadDirectoryChangesW error:", std::system_category().message(static_cast<int>(GetLastError())));
        return;
    }

    SetEvent(mEvents.front());  // Send kill signal to reflect the change
}
