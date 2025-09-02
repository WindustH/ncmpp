
/**
 * @file utils.cpp
 * @brief Utility functions for NCM file processing
 * @details Provides helper functions for hex conversion and little-endian integer handling
 */

#include "utils.h"
#include <sstream>
#include <iostream>

namespace ncm {
namespace utils {

/**
 * @brief Convert hex string to binary data
 * @param src Hex string (32 characters for 16 bytes)
 * @param dest Output buffer (must be at least 16 bytes)
 * @details Converts pairs of hex characters to binary bytes
 * Example: "687A" -> [0x68, 0x7A]
 */
void hex2str(const char* src, unsigned char* dest) {
    if (!src || !dest) {
        std::cerr << "[ERROR] hex2str: Invalid input parameters" << std::endl;
        return;
    }
    
    std::string hs(src, 32);
    std::stringstream ss;
    
    for (int i = 0; i < 16; i++) {
        ss.clear();
        ss.str(hs.substr(i * 2, 2));
        unsigned int value = 0;
        ss >> std::hex >> value;
        dest[i] = static_cast<unsigned char>(value);
        
        if (ss.fail()) {
            std::cerr << "[ERROR] hex2str: Failed to parse hex at position " << i << std::endl;
            dest[i] = 0;
        }
    }
}

/**
 * @brief Convert little-endian bytes to unsigned int
 * @param src Pointer to 4 bytes of little-endian data
 * @return Converted unsigned integer value
 * @details Handles endian conversion from little-endian format
 */
unsigned int little_int(const unsigned char* src) {
    if (!src) {
        std::cerr << "[ERROR] little_int: NULL input pointer" << std::endl;
        return 0;
    }
    
    unsigned int result = 0;
    for (int i = 3; i >= 0; i--) {
        result <<= 8;
        result += src[i];
    }
    
    return result;
}

}
}
