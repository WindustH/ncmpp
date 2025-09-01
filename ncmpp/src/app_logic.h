#pragma once
#include "app_config.h"
#include <atomic>
#include <filesystem>

class ncm_app {
public:
    explicit ncm_app(app_config config);
    int run();

private:
    void run_batch_mode();
    void run_fallback_mode();
    void setup_logging() const;
    void process_file(const std::filesystem::path& input_path, const std::filesystem::path& output_path);

    app_config config_;
    std::atomic<int> total_pieces_ = 0;
};