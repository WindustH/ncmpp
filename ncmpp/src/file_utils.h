#pragma once
#include <filesystem>
#include <vector>
#include <string>

inline std::vector<std::filesystem::path> find_files(const std::filesystem::path& dir, const std::string& extension) {
    std::vector<std::filesystem::path> files;
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
        return files;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            files.push_back(entry.path());
        }
    }

    return files;
}
