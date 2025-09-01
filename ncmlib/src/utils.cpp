
#include "utils.h"
#include <sstream>

namespace ncm {
namespace utils {

void hex2str(const char* src, unsigned char* dest) {
    std::string hs(src, 32);
    std::string bs;
    std::stringstream ss;
    unsigned int is;
    for (int i = 0; i < 16; i++) {
        ss.clear();
        bs = hs.substr(i * 2, 2);
        ss << std::hex << bs;
        ss >> is;
        dest[i] = is;
    }
}

unsigned int little_int(const unsigned char* src) {
    unsigned int ret = 0;
    for (int i = 3; i >= 0; i--) {
        ret <<= 8;
        ret += src[i];
    }
    return ret;
}

}
}
