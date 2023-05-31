#include "../../low_level/file_change_watcher.h"

#include <fstream>

#include <Windows.h>

#include "../../utils/logger.h"
#include "log.h"


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
                    log_last_error(L"WaitForMultipleObjectsEx error:");
                    return;
                }
                
                if (result != WAIT_OBJECT_0)  // not a "kill" signal
                {
                    const int directoryIndex = static_cast<int>(result - WAIT_OBJECT_0 - 1);
                    const std::wstring& targetFileName = mFileNames.at(directoryIndex);

                    const auto* info = static_cast<FILE_NOTIFY_INFORMATION*>(mBuffers.at(directoryIndex));
                    while (true)
                    {
                        const std::wstring_view fileName{ info->FileName, info->FileNameLength / sizeof(WCHAR) };
                        if (fileName == targetFileName)
                        {
                            // The file change watcher gets triggered twice when the file is modified.
                            // For the first trigger, it is possible that the file is locked by the program that modifies it.
                            // So check if we can open the file before calling onChanged().
                            std::wifstream configFile{ mFullPaths.at(directoryIndex), std::ios::binary | std::ios::ate};
                            if (configFile.tellg() != 0)
                            {
                                onChanged();
                            }
                            break;
                        }

                        if (info->NextEntryOffset <= 0)
                        {
                            break;
                        }
                        info = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(reinterpret_cast<const char*>(info) + info->NextEntryOffset);
                    }

                    readDirectoryChanges(directoryIndex);
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
    for (const void* bufferPtr : mBuffers)
    {
        const auto* buffer = static_cast<const char*>(bufferPtr);
        delete[] buffer;
    }
    for (const void* overlappedPtr : mOverlappeds)
    {
        const auto* overlapped = static_cast<const OVERLAPPED*>(overlappedPtr);
        delete overlapped;
    }
}


void FileChangeWatcher::AddWatchingFile(const std::filesystem::path& filePath)
{
    const HANDLE dir = CreateFile(filePath.parent_path().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
    if (dir == INVALID_HANDLE_VALUE) [[unlikely]]
    {
        log_last_error(L"CreateFile error:");
        return;
    }
    mFullPaths.emplace_back(filePath);
    mFileNames.emplace_back(filePath.filename().wstring());
    mDirectories.emplace_back(dir);

    const auto overlapped = new OVERLAPPED{};
    overlapped->hEvent = CreateEvent(nullptr, false, false, nullptr);
    mEvents.emplace_back(overlapped->hEvent);
    mBuffers.emplace_back(new char[512]);
    mOverlappeds.emplace_back(overlapped);

    readDirectoryChanges(static_cast<int>(mDirectories.size() - 1));
}


void FileChangeWatcher::readDirectoryChanges(int index) const
{
    if (!ReadDirectoryChangesW(mDirectories.at(index), mBuffers.at(index), 512, false, FILE_NOTIFY_CHANGE_LAST_WRITE, nullptr,
        static_cast<OVERLAPPED*>(mOverlappeds.at(index)), nullptr))
    {
        log_last_error(L"ReadDirectoryChangesW error:");
        return;
    }

    SetEvent(mEvents.front());  // Send kill signal to reflect the change
}
