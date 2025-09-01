
#include "ncmlib/ncmdump.h"
#include "NcmFile.h"
#include <stdexcept>
#include <iostream>

void ncm::ncmDump(std::string path, std::string outPath) {
    try {
        ncm::NcmFile ncm_file(path);
        ncm_file.dump(outPath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
