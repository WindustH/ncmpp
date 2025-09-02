/**
 * @file file_utils.h
 * @brief File system utilities for ncmpp
 * @details Provides helper functions for file discovery and path handling
 */

#pragma once
#include <filesystem>
#include <vector>
#include <string>

/**
 * @brief Recursively find files with specific extension
 * @param dir Directory path to search in
 * @param extension File extension to match (including the dot, e.g., ".ncm")
 * @return Vector of file paths matching the extension
 * @details Performs recursive directory traversal to find all files
 * with the specified extension. Handles edge cases like:
 * - Non-existent directories
 * - Non-directory paths
 * - Empty results gracefully
 */
inline std::vector<std::filesystem::path> find_files(const std::filesystem::path& dir, const std::string& extension) {
    std::vector<std::filesystem::path> files;
    
    // Validate input directory
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
        return files;
    }

    try {
        // Recursive directory traversal
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == extension) {
                files.push_back(entry.path());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem errors gracefully
        return files;
    }

    return files;
}
