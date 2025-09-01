
#pragma once
#include <string>
#include <vector>
#include <filesystem>

struct app_config {
    unsigned int thread_count;
    bool show_time;
    std::string input_file_list;
    std::string output_file_list;
    std::filesystem::path output_dir;
};
