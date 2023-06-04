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
    };
    std::vector<void*> mEvents;
    std::vector<File> mFiles;
};
