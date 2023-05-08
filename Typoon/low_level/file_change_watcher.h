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

    std::vector<std::wstring> mFileNames;
    std::vector<void*> mDirectories;
    std::vector<void*> mEvents;
    std::vector<void*> mBuffers;
    std::vector<void*> mOverlappeds;
};
