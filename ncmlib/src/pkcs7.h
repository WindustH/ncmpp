/**
 * @file pkcs7.h
 * @brief PKCS#7 padding interface for NCM file processing
 * @details Header file for PKCS#7 padding removal functionality
 * @note Used for removing padding from AES-decrypted data in NCM files
 */

#pragma once

namespace pkcs7 {

/**
 * @brief Calculate the actual data size after PKCS#7 padding removal
 * @param src_ Pointer to the padded data buffer
 * @param len_ Length of the padded data buffer
 * @return Actual unpadded data size
 * @throws std::runtime_error if padding is invalid
 */
unsigned int pad_size(const unsigned char* src_, unsigned int len_);

/**
 * @brief Remove PKCS#7 padding from decrypted data
 * @param src_ Pointer to the padded source data
 * @param len_ Length of the source data
 * @param tgt_ Output buffer for unpadded data
 * @throws std::runtime_error if padding is invalid
 */
void unpad(const unsigned char* src_, unsigned int len_, unsigned char* tgt_);

} // namespace pkcs7
