/**
 * @file ncmdump.cpp
 * @brief Main NCM file processing interface
 * @details Provides the public API entry point for NCM file decryption and extraction
 * @note This is the main interface exposed to external applications
 */

#include "ncmlib/ncmdump.h"
#include "NcmFile.h"
#include <stdexcept>
#include <iostream>

namespace ncm {

/**
 * @brief Decrypt and extract audio from an NCM file
 * @param path Path to the input .ncm file
 * @param outPath Output path for the decrypted file (without extension)
 * @details This is the main entry point for NCM file processing:
 * 1. Opens the NCM file
 * 2. Decrypts the audio content
 * 3. Extracts metadata and cover image
 * 4. Saves the decrypted audio file
 * @throws std::exception if file processing fails
 */
void ncmDump(std::string path, std::string outPath) {
    try {
        std::cout << "[INFO] Processing NCM file: " << path << std::endl;
        std::cout << "[INFO] Output path: " << outPath << std::endl;
        
        NcmFile ncm_file(path);
        ncm_file.dump(outPath);
        
        std::cout << "[SUCCESS] Successfully processed: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to process " << path << ": " << e.what() << std::endl;
        throw; // Re-throw to allow caller to handle
    }
}

} // namespace ncm
