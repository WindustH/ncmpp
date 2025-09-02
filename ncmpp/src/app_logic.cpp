#include "app_logic.h"
#include "ncmlib/ncmdump.h"
#include "pool.h"
#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <mutex>
#include <functional>

using namespace std;

namespace {
    /**
     * @brief Thread-safe logging utility
     * @note Uses mutex to ensure thread-safe console output
     */
    mutex log_mtx;

    /**
     * @brief Log message to console with thread safety
     * @param message Message to log
     * @param level Log level (INFO, WARN, ERROR, DEBUG)
     */
    void log(const string& message, const string& level = "INFO") {
        lock_guard<mutex> lock(log_mtx);
        
        // Color codes for different log levels
        string color_code;
        if (level == "ERROR") color_code = "\033[31m";
        else if (level == "WARN") color_code = "\033[33m";
        else if (level == "DEBUG") color_code = "\033[36m";
        else color_code = "\033[32m";
        
        cout << color_code << "[" << level << "] " << message << "\033[0m" << endl;
    }

    /**
     * @brief Read lines from text file
     * @param file_path Path to the file to read
     * @return Vector of non-empty lines
     * @note Automatically skips empty lines and handles file errors
     */
    vector<string> read_file_lines(const string& file_path) {
        vector<string> lines;
        ifstream file(file_path);
        if (file.is_open()) {
            string line;
            int line_count = 0;
            while (getline(file, line)) {
                line_count++;
                if (!line.empty()) {
                    lines.push_back(line);
                }
            }
            file.close();
            log("Read " + to_string(lines.size()) + " lines from " + file_path + " (" + to_string(line_count) + " total)");
        } else {
            log("Error: Unable to open file: " + file_path, "ERROR");
        }
        return lines;
    }
} // anonymous namespace

ncm_app::ncm_app(app_config config) : config_(std::move(config)) {}

/**
 * @brief Run the main application logic
 * @return Exit code (0 for success)
 * @details Processes NCM files either in batch mode (using file lists) or
 * fallback mode (processing individual .ncm files)
 */
int ncm_app::run() {
    setup_logging();
    log("Starting NCM processing with " + to_string(config_.thread_count) + " threads");
    log("Configuration:");
    log("  Input file: " + (config_.input_file_list.empty() ? "<auto-detect>" : config_.input_file_list));
    log("  Output file: " + (config_.output_file_list.empty() ? config_.output_dir.string() : config_.output_file_list));
    log("  Show timing: " + string(config_.show_time ? "true" : "false"));

    auto start = chrono::steady_clock::now();

    try {
        if (!config_.input_file_list.empty() && !config_.output_file_list.empty()) {
            log("Running in batch mode with file lists");
            run_batch_mode();
        } else {
            log("Running in fallback mode with directory scanning");
            run_fallback_mode();
        }

        auto end = chrono::steady_clock::now();
        double elapsed_seconds = chrono::duration_cast<chrono::milliseconds>(end - start).count() / 1000.0;
        
        log("Processing complete!");
        log("Total files processed: " + to_string(total_pieces_));
        
        if (config_.show_time) {
            log("Total time elapsed: " + to_string(elapsed_seconds) + "s");
        }
        
        return 0;
        
    } catch (const exception& e) {
        log("Fatal error: " + string(e.what()), "ERROR");
        return 1;
    }
}

/**
 * @brief Process a single NCM file
 * @param input_path Path to the input .ncm file
 * @param output_path Path where the decrypted file should be written
 * @details Handles the processing of individual NCM files with error handling
 * and progress reporting.
 */
void ncm_app::process_file(const filesystem::path& input_path, const filesystem::path& output_path) {
    try {
        filesystem::path output_dir = output_path.parent_path();
        if (!output_dir.empty() && !filesystem::exists(output_dir)) {
            filesystem::create_directories(output_dir);
        }

        log("Processing: " + input_path.filename().string());
        
        auto start_time = chrono::steady_clock::now();
        ncm::ncmDump(input_path.string(), output_path.string());
        
        auto end_time = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
        
        log("Completed: " + input_path.filename().string() + " (" + to_string(duration) + "ms)");
        total_pieces_++;
        
    } catch (const exception& e) {
        log("Error processing " + input_path.string() + ": " + e.what(), "ERROR");
    }
}

/**
 * @brief Run in batch mode using input/output file lists
 * @details Processes files using lists provided via -i and -o flags
 */
void ncm_app::run_batch_mode() {
    vector<string> input_files = read_file_lines(config_.input_file_list);
    vector<string> output_files = read_file_lines(config_.output_file_list);

    if (input_files.empty() || output_files.empty()) {
        log("Input or output file list is empty.", "ERROR");
        return;
    }

    if (input_files.size() != output_files.size()) {
        log("Input file count: " + to_string(input_files.size()) + ", Output file count: " + to_string(output_files.size()), "ERROR");
        log("Error: Input and output file lists must have the same number of lines.", "ERROR");
        return;
    }

    log("Processing " + to_string(input_files.size()) + " files in batch mode");
    
    thread_pool pool(config_.thread_count);
    for (size_t i = 0; i < input_files.size(); ++i) {
        pool.enqueue(std::bind(&ncm_app::process_file, this, input_files[i], output_files[i]));
    }
    
    log("All tasks queued, waiting for completion...");
}

/**
 * @brief Run in fallback mode using directory scanning
 * @details Scans directory for .ncm files and processes them automatically
 */
void ncm_app::run_fallback_mode() {
    if (!filesystem::exists(config_.output_dir)) {
        log("Creating output directory: " + config_.output_dir.string());
        filesystem::create_directory(config_.output_dir);
    }

    vector<filesystem::path> files_to_process = find_files(".", ".ncm");
    if (files_to_process.empty()) {
        log("No .ncm files found to process.", "WARN");
        return;
    }

    log("Found " + to_string(files_to_process.size()) + " .ncm files to process");
    
    thread_pool pool(config_.thread_count);
    for (const auto& file_path : files_to_process) {
        filesystem::path output_path = config_.output_dir / file_path.stem();
        pool.enqueue(std::bind(&ncm_app::process_file, this, file_path, output_path));
    }
    
    log("All tasks queued for " + to_string(files_to_process.size()) + " files");
}

void ncm_app::setup_logging() const {
    // In the future, we could set up a more sophisticated logging system here.
    // For now, we just use stdout.
}