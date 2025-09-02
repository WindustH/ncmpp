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
    mutex log_mtx;

    void log(const string& message) {
        lock_guard<mutex> lock(log_mtx);
        cout << message << endl;
    }

    vector<string> read_file_lines(const string& file_path) {
        vector<string> lines;
        ifstream file(file_path);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                if (!line.empty()) {
                    lines.push_back(line);
                }
            }
            file.close();
        } else {
            log("\033[31mError: Unable to open file: " + file_path + "\033[0m");
        }
        return lines;
    }
} // anonymous namespace

ncm_app::ncm_app(app_config config) : config_(std::move(config)) {}

int ncm_app::run() {
    setup_logging();
    log("Starting with " + to_string(config_.thread_count) + " threads.\n");

    auto start = chrono::steady_clock::now();

    if (!config_.input_file_list.empty() && !config_.output_file_list.empty()) {
        run_batch_mode();
    } else {
        run_fallback_mode();
    }

    auto end = chrono::steady_clock::now();
    log("\n\033[32mFinished.\033[0m");
    log("Unlocked " + to_string(total_pieces_) + " pieces of music.");

    if (config_.show_time) {
        log("Time elapsed: " + to_string(chrono::duration_cast<chrono::milliseconds>(end - start).count() / 1000.0) + "s");
    }

    return 0;
}

void ncm_app::process_file(const filesystem::path& input_path, const filesystem::path& output_path) {
    try {
        filesystem::path output_dir = output_path.parent_path();
        if (!output_dir.empty() && !filesystem::exists(output_dir)) {
            filesystem::create_directories(output_dir);
        }

        ncm::ncmDump(input_path.string(), output_path.string());
        log("\033[36mUnlocked:\t" + input_path.filename().string() + " -> " + output_path.string() + "\033[0m");
        total_pieces_++;
    } catch (const exception& e) {
        log("\033[31mError processing " + input_path.string() + ": " + e.what() + "\033[0m");
    }
}

void ncm_app::run_batch_mode() {
    vector<string> input_files = read_file_lines(config_.input_file_list);
    vector<string> output_files = read_file_lines(config_.output_file_list);

    if (input_files.empty() || output_files.empty()) {
        log("Input or output file list is empty.");
        return;
    }

    if (input_files.size() != output_files.size()) {
        log("\033[31mError: Input and output file lists must have the same number of lines.\033[0m");
        return;
    }

    thread_pool pool(config_.thread_count);
    for (size_t i = 0; i < input_files.size(); ++i) {
        pool.enqueue(std::bind(&ncm_app::process_file, this, input_files[i], output_files[i]));
    }
}

void ncm_app::run_fallback_mode() {
    if (!filesystem::exists(config_.output_dir)) {
        filesystem::create_directory(config_.output_dir);
    }

    vector<filesystem::path> files_to_process = find_files(".", ".ncm");
    if (files_to_process.empty()) {
        log("No .ncm files found to process.");
        return;
    }

    thread_pool pool(config_.thread_count);
    for (const auto& file_path : files_to_process) {
        filesystem::path output_path = config_.output_dir / file_path.stem();
        pool.enqueue(std::bind(&ncm_app::process_file, this, file_path, output_path));
    }
}

void ncm_app::setup_logging() const {
    // In the future, we could set up a more sophisticated logging system here.
    // For now, we just use stdout.
}