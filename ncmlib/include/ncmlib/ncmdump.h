/**
 * @file ncmdump.h
 * @brief Public API for NCM file processing
 * @details Header file providing the main interface for NCM file decryption
 * @note This is the public API exposed by the ncmpp library
 */

#pragma once

#include <string>

namespace ncm {

/**
 * @brief Decrypt and extract audio from an NCM file
 * @param path Path to the input .ncm file
 * @param outPath Output path for the decrypted file (without extension)
 * @details This is the main entry point for NCM file processing that handles:
 * - File format validation and parsing
 * - AES decryption of audio content
 * - Metadata extraction
 * - Cover image extraction (if available)
 * - Output file generation with correct extension
 * @throws std::exception if file processing fails
 */
void ncmDump(std::string path, std::string outPath);

} // namespace ncm
