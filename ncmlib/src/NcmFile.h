
#pragma once
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include "rapidjson/document.h"

namespace ncm {

class NcmFile {
public:
    NcmFile(const std::filesystem::path& path);
    void dump(const std::filesystem::path& out_path);

private:
    void _read_key_data();
    void _setup_key_box();
    void _read_metadata();
    void _parse_metadata();
    void _dump_audio_data(const std::filesystem::path& out_path);

    std::filesystem::path _path;
    std::ifstream _file;
    std::vector<unsigned char> _key_data;
    std::vector<unsigned char> _key_box;
    rapidjson::Document _metadata;
};

}
