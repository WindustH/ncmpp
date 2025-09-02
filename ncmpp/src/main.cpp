
/**
 * @file main.cpp
 * @brief Main entry point for ncmpp command-line application
 * @details Provides command-line interface for batch NCM file processing
 * @note Supports both batch mode (with file lists) and fallback mode (directory scanning)
 */

#include "cmdline.h"
#include "app_config.h"
#include "app_logic.h"
#include <thread>
#include <iostream>

/**
 * @brief Main entry point for ncmpp application
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code (0 for success, 1 for error)
 * @details Processes command-line arguments and runs the NCM file processor in either:
 * - Batch mode: Using input/output file lists specified with -i and -o
 * - Fallback mode: Scanning directory for .ncm files when no input list provided
 */
int main(int argc, char* argv[]) {
    try {
        cmdline::parser cmd;
        cmd.set_program_name("ncmpp");
        
        // Thread count option
        cmd.add<unsigned int>("threads", 't', 
            "Maximum number of concurrent processing threads", 
            false, std::thread::hardware_concurrency());
        
        // Timing option
        cmd.add("showtime", 's', 
            "Display processing time for each file and total time");
        
        // Input file list option
        cmd.add<std::string>("input", 'i', 
            "Path to text file containing list of input .ncm files", 
            false, "");
        
        // Output option (file list or directory)
        cmd.add<std::string>("output", 'o', 
            "Path to text file for output file list (batch mode) or directory for fallback mode", 
            false, "unlocked");
        
        // Help option
        cmd.add("help", 'h', "Print this help message");

        cmd.parse_check(argc, argv);

        // Configure application
        app_config config;
        config.thread_count = cmd.get<unsigned int>("threads");
        config.show_time = cmd.exist("showtime");
        config.input_file_list = cmd.get<std::string>("input");

        // Determine output mode based on input file list
        std::string output_path_str = cmd.get<std::string>("output");
        if (!config.input_file_list.empty()) {
            // Batch mode: output is a file list
            config.output_file_list = output_path_str;
        } else {
            // Fallback mode: output is a directory
            config.output_dir = output_path_str;
        }

        // Validate thread count
        if (config.thread_count == 0) {
            std::cerr << "[ERROR] Thread count must be at least 1" << std::endl;
            return 1;
        }

        // Run the application
        ncm_app app(config);
        return app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[FATAL ERROR] Unknown exception occurred" << std::endl;
        return 1;
    }
}
