#pragma once
#include <filesystem>
#include <functional>
#include <thread>


class [[nodiscard]] FileChangeWatcher
{
public:
    FileChangeWatcher(std::function<void()> onChanged);
    ~FileChangeWatcher();
    FileChangeWatcher(const FileChangeWatcher& other) = delete;
    FileChangeWatcher(FileChangeWatcher&& other) noexcept = delete;
    FileChangeWatcher& operator=(const FileChangeWatcher& other) = delete;
    FileChangeWatcher& operator=(FileChangeWatcher&& other) noexcept = delete;

    void AddWatchingFile(const std::filesystem::path& filePath);

private:
    void readDirectoryChanges(int index) const;


private:
    std::jthread mThread;

    struct File
    {
        std::filesystem::path fullPath;
        std::wstring fileName;
        void* directory = nullptr;
        void* buffer = nullptr;
        void* overlapped = nullptr;
        std::chrono::time_point<std::chrono::system_clock> lastWriteTime;
    };
    std::vector<void*> mEvents;  // The first one is the 'kill' event, not an actual file's event.
    std::vector<File> mFiles;  // This one contains only the actual files. Therefore, mFiles.at(n) == mEvents.at(n + 1).

    static constexpr std::chrono::milliseconds WRITE_COOLDOWN = std::chrono::milliseconds(100);
};
