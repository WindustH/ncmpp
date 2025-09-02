/**
 * @file pkcs7.cpp
 * @brief PKCS#7 padding implementation for NCM file processing
 * @details Provides PKCS#7 (RFC 5652) padding removal functionality for AES-decrypted data
 * @note Used specifically for removing padding from decrypted key and metadata in NCM files
 */

#include "pkcs7.h"
#include <stdexcept>

namespace pkcs7 {

/**
 * @brief Calculate the actual data size after PKCS#7 padding removal
 * @param src_ Pointer to the padded data buffer
 * @param len_ Length of the padded data buffer
 * @return Actual unpadded data size
 * @throws std::runtime_error if padding is invalid
 * @details PKCS#7 padding adds N bytes of value N to reach block boundary
 */
unsigned int pad_size(const unsigned char* src_, unsigned int len_) {
    if (len_ == 0) {
        return 0;
    }
    
    unsigned int padlen = src_[len_ - 1];
    
    // Validate padding - padlen should be between 1 and 16 for AES-128
    if (padlen == 0 || padlen > 16) {
        throw std::runtime_error("Invalid PKCS#7 padding length");
    }
    
    // Validate all padding bytes have the same value
    for (unsigned int i = len_ - padlen; i < len_; i++) {
        if (src_[i] != padlen) {
            throw std::runtime_error("Invalid PKCS#7 padding bytes");
        }
    }
    
    return len_ - padlen;
}

/**
 * @brief Remove PKCS#7 padding from decrypted data
 * @param src_ Pointer to the padded source data
 * @param len_ Length of the source data
 * @param tgt_ Output buffer for unpadded data (must be pre-allocated)
 * @throws std::runtime_error if padding is invalid
 * @details Copies the actual data without padding bytes to the target buffer
 */
void unpad(const unsigned char* src_, unsigned int len_, unsigned char* tgt_) {
    unsigned int size = pad_size(src_, len_);
    for (unsigned int i = 0; i < size; i++) {
        tgt_[i] = src_[i];
    }
}

} // namespace pkcs7
