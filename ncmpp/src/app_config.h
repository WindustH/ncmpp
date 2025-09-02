
/**
 * @file app_config.h
 * @brief Application configuration structure for ncmpp
 * @details Defines configuration parameters for the NCM file processor
 */

#pragma once
#include <string>
#include <filesystem>

/**
 * @brief Configuration structure for ncmpp application
 * @details Contains all runtime configuration parameters including:
 * - Thread count for concurrent processing
 * - Timing display preference
 * - Input/output file list paths for batch mode
 * - Output directory for fallback mode
 */
struct app_config {
    /** @brief Number of threads for concurrent processing */
    unsigned int thread_count;
    
    /** @brief Whether to display processing time information */
    bool show_time;
    
    /** @brief Path to input file list (batch mode) */
    std::string input_file_list;
    
    /** @brief Path to output file list (batch mode) */
    std::string output_file_list;
    
    /** @brief Output directory path (fallback mode) */
    std::filesystem::path output_dir;
};
