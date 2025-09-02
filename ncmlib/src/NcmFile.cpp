/**
 * @file NcmFile.cpp
 * @brief NCM file format parser and decoder
 * @details This file implements the NCM file format parser which can decrypt
 * and extract audio data from NetEase Cloud Music (.ncm) files.
 */

#include "NcmFile.h"
#include "utils.h"
#include "base64.h"
#include "pkcs7.h"
#include "openssl/evp.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

using namespace std;

// ANSI color codes
const char* RESET = "\033[0m";
const char* BLUE = "\033[34m";
const char* GREEN = "\033[32m";
const char* YELLOW = "\033[33m";
const char* RED = "\033[31m";
const char* CYAN = "\033[36m";
using namespace ncm;
using namespace ncm::utils;

namespace {
    /**
     * @brief AES-128 ECB mode decryption key for core data
     * @note This is a fixed key used for decrypting the core key data in NCM files
     */
    const char CORE_HEX_KEY[] = "687A4852416D736F356B496E62617857";
    
    /**
     * @brief AES-128 ECB mode decryption key for metadata
     * @note This is a fixed key used for decrypting the metadata in NCM files
     */
    const char META_HEX_KEY[] = "2331346C6A6B5F215C5D2630553C2728";

    /**
     * @brief Decrypt data using AES-128 ECB mode
     * @param ciphertext The encrypted data
     * @param ciphertext_len Length of encrypted data
     * @param key AES-128 key (16 bytes)
     * @return Decrypted data as unsigned char vector
     * @throws runtime_error if decryption fails
     */
    vector<unsigned char> aes_ecb_decrypt(const unsigned char* ciphertext, int ciphertext_len, const unsigned char* key) {
        EVP_CIPHER_CTX* ctx = nullptr;
        vector<unsigned char> plaintext(ciphertext_len);
        int len;
        int plaintext_len;

        try {
            // Create new cipher context
            if (!(ctx = EVP_CIPHER_CTX_new())) {
                throw runtime_error("Failed to create new EVP cipher context");
            }

            // Initialize decryption with AES-128 ECB
            if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL)) {
                throw runtime_error("Failed to initialize EVP decryption");
            }

            // Disable padding since NCM uses fixed-size blocks
            EVP_CIPHER_CTX_set_padding(ctx, 0);

            // Decrypt the data
            if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext, ciphertext_len)) {
                throw runtime_error("Failed to update EVP decryption");
            }
            plaintext_len = len;

            // Finalize decryption
            if (1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len)) {
                throw runtime_error("Failed to finalize EVP decryption");
            }
            plaintext_len += len;

            EVP_CIPHER_CTX_free(ctx);

            // Resize to actual decrypted data size
            plaintext.resize(plaintext_len);
            return plaintext;
            
        } catch (...) {
            if (ctx) EVP_CIPHER_CTX_free(ctx);
            throw;
        }
    }
} // anonymous namespace

/**
 * @brief Construct NcmFile object from filesystem path
 * @param path Path to the .ncm file to process
 * @throws runtime_error if file cannot be opened
 */
NcmFile::NcmFile(const filesystem::path& path) : _path(path) {
    _file.open(_path, ios::in | ios::binary);
    if (!_file.is_open()) {
        throw runtime_error("Failed to open file: " + path.string());
    }
    
    std::cout << "[INFO] Opening NCM file: " << BLUE << path.string() << RESET << std::endl;
}

/**
 * @brief Dump decrypted audio and cover image from NCM file
 * @param out_path Output path for the decrypted audio file (without extension)
 * @details This method performs the complete NCM file processing:
 * 1. Reads and decrypts the key data
 * 2. Sets up the key box for audio decryption
 * 3. Reads and parses metadata
 * 4. Extracts cover image if available
 * 5. Decrypts and writes the audio data
 */
void NcmFile::dump(const filesystem::path& out_path) {
    std::cout << "[INFO] Processing NCM file: " << BLUE << _path.filename() << RESET << std::endl;
    
    try {
        _read_key_data();
        _setup_key_box();
        _read_metadata();
        _parse_metadata();
        _dump_audio_data(out_path);
        
        std::cout << GREEN << "[SUCCESS] Successfully processed: " << RESET << BLUE << _path.filename() << RESET << std::endl;
    } catch (const exception& e) {
        std::cerr << RED << "[ERROR] Failed to process " << RESET << BLUE << _path.filename() << RESET << ": " << e.what() << std::endl;
        throw;
    }
}

/**
 * @brief Read and decrypt the key data from NCM file
 * @details Reads the encrypted key data from the NCM file structure:
 * - Skips the first 10 bytes (header)
 * - Reads 4-byte key length
 * - Reads key data and applies XOR 0x64
 * - Decrypts using AES-128 ECB with fixed key
 * - Removes PKCS7 padding
 */
void NcmFile::_read_key_data() {
    std::cout << CYAN << "[DEBUG] Reading key data..." << RESET << std::endl;
    
    // Skip 10 bytes of file header
    _file.seekg(10, ios::cur);
    
    // Read key length (4 bytes, little-endian)
    unsigned char key_len_bin[4];
    _file.read((char*)key_len_bin, 4);
    unsigned int key_len = little_int(key_len_bin);
    
    std::cout << CYAN << "[DEBUG] Key data length: " << key_len << " bytes" << RESET << std::endl;

    // Read encrypted key data
    vector<unsigned char> key_data_bin(key_len);
    _file.read((char*)key_data_bin.data(), key_len);
    
    // Apply XOR 0x64 to each byte
    for (unsigned int i = 0; i < key_len; i++) {
        key_data_bin[i] ^= 0x64;
    }

    // Decrypt using AES-128 ECB
    unsigned char core_key[16];
    hex2str(CORE_HEX_KEY, core_key);
    vector<unsigned char> decrypted_key = aes_ecb_decrypt(key_data_bin.data(), key_len, core_key);

    // Remove PKCS7 padding
    unsigned int unpadded_len = pkcs7::pad_size(decrypted_key.data(), decrypted_key.size());
    _key_data.resize(unpadded_len);
    pkcs7::unpad(decrypted_key.data(), decrypted_key.size(), _key_data.data());
    
    std::cout << CYAN << "[DEBUG] Successfully decrypted key data, length: " << unpadded_len << " bytes" << RESET << std::endl;
}

/**
 * @brief Setup the key box for audio data decryption
 * @details Implements the RC4-like key scheduling algorithm to generate
 * a 256-byte key box used for decrypting the audio stream
 */
void NcmFile::_setup_key_box() {
    std::cout << "[DEBUG] Setting up key box..." << std::endl;
    
    // Skip the first 17 bytes of key data (header)
    unsigned char* key_data_use = _key_data.data() + 17;
    unsigned int key_len_unpad = _key_data.size();

    // Initialize key box with identity permutation
    _key_box.resize(256);
    for (unsigned int i = 0; i < 256; i++) {
        _key_box[i] = i;
    }

    // RC4-like key scheduling algorithm
    unsigned char c = 0;
    unsigned char last_byte = 0;
    unsigned int key_offset = 0;
    unsigned char swap = 0;
    
    for (unsigned int i = 0; i < 256; i++) {
        swap = _key_box[i];
        c = (swap + last_byte + key_data_use[key_offset]) & 0xff;
        key_offset++;
        if (key_offset >= key_len_unpad - 17) {
            key_offset = 0;
        }
        _key_box[i] = _key_box[c];
        _key_box[c] = swap;
        last_byte = c;
    }
    
    std::cout << CYAN << "[DEBUG] Key box setup complete" << RESET << std::endl;
}

void NcmFile::_read_metadata() {
    unsigned char mata_len_bin[4];
    _file.read((char*)mata_len_bin, 4);
    unsigned int mata_len = little_int(mata_len_bin);
    if (mata_len == 0) return; // No metadata

    vector<unsigned char> mata_data_bin(mata_len);
    _file.read((char*)mata_data_bin.data(), mata_len);

    for (unsigned int i = 0; i < mata_len; i++) {
        mata_data_bin[i] ^= 0x63;
    }

    string mata_data_base64((char*)mata_data_bin.data() + 22, mata_len - 22);
    string mata_data_str = base64_decode(mata_data_base64);

    unsigned char mata_key[16];
    hex2str(META_HEX_KEY, mata_key);
    vector<unsigned char> mata_data_decrypted = aes_ecb_decrypt((const unsigned char*)mata_data_str.c_str(), mata_data_str.length(), mata_key);

    unsigned int mata_len_unpad = pkcs7::pad_size(mata_data_decrypted.data(), mata_data_decrypted.size());
    vector<unsigned char> mata_data(mata_len_unpad);
    pkcs7::unpad(mata_data_decrypted.data(), mata_data_decrypted.size(), mata_data.data());

    string mata_str((char*)mata_data.data() + 6, mata_len_unpad - 6);
    _metadata.Parse(mata_str.c_str(), mata_str.length());
}

void NcmFile::_parse_metadata() {
    // This is already done in _read_metadata. 
    // This method could be used for further processing if needed.
}

/**
 * @brief Dump decrypted audio and cover image from NCM file
 * @param out_path Output path for the decrypted audio file (without extension)
 * @details This function:
 * 1. Extracts cover image if present
 * 2. Determines output format from metadata
 * 3. Decrypts and writes the audio data using the key box
 * 4. Handles progress reporting and error handling
 */
void NcmFile::_dump_audio_data(const filesystem::path& out_path) {
    std::cout << CYAN << "[DEBUG] Extracting audio and cover data..." << RESET << std::endl;
    
    // Skip 9 bytes (CRC checksum)
    _file.seekg(9, ios::cur);
    
    // Read cover image length
    unsigned char image_len_bin[4];
    _file.read((char*)image_len_bin, 4);
    unsigned int image_len = little_int(image_len_bin);

    if (image_len > 0) {
        std::cout << CYAN << "[DEBUG] Found cover image, size: " << image_len << " bytes" << RESET << std::endl;
        
        vector<unsigned char> image_data(image_len);
        _file.read((char*)image_data.data(), image_len);

        filesystem::path cover_path = out_path;
        cover_path += ".jpg";

        // Ensure directory exists
        if (cover_path.has_parent_path()) {
            filesystem::create_directories(cover_path.parent_path());
        }

        // Write cover image
        ofstream cover_of(cover_path, ios::out | ios::binary);
        if (cover_of.is_open()) {
            cover_of.write((char*)image_data.data(), image_len);
            cover_of.close();
            std::cout << "[INFO] Cover image extracted: " << BLUE << cover_path.filename() << RESET << std::endl;
        } else {
            std::cerr << "[WARNING] Failed to write cover image: " << cover_path << std::endl;
        }
    } else {
        std::cout << CYAN << "[DEBUG] No cover image found" << RESET << std::endl;
        _file.seekg(image_len, ios::cur);
    }

    // Determine output file extension from metadata
    string extname = "." + string(_metadata["format"].GetString());
    filesystem::path tgt = out_path;
    
    // Add extension directly to preserve full filename including dots
    tgt += extname;

    std::cout << "[INFO] Writing audio file: " << BLUE << tgt.filename() << RESET << std::endl;

    // Ensure directory exists
    if (tgt.has_parent_path()) {
        filesystem::create_directories(tgt.parent_path());
    }

    // Open output file
    ofstream of(tgt, ios::out | ios::binary);
    if (!of.is_open()) {
        throw runtime_error("Failed to open output file: " + tgt.string());
    }

    // Process audio data with clean progress bar
    unsigned char buff[0x8000]; // 32KB buffer
    uint64_t total_bytes = 0;
    
    auto progress_start = chrono::steady_clock::now();
    uint64_t last_reported_bytes = 0;
    
    _file.read((char*)buff, sizeof(buff));
    unsigned int buff_len = _file.gcount();
    
    while (buff_len > 0) {
        // Apply decryption using key box
        for (unsigned int i = 0; i < buff_len; i++) {
            int j = (i + 1) & 0xff;
            buff[i] ^= _key_box[(_key_box[j] + _key_box[(_key_box[j] + j) & 0xff]) & 0xff];
        }
        
        // Write decrypted data
        of.write((char*)buff, buff_len);
        total_bytes += buff_len;
        
        // Clean progress indicator - show dots every 10MB
        if (total_bytes - last_reported_bytes >= 10 * 1024 * 1024) {
            std::cout << "." << std::flush;
            last_reported_bytes = total_bytes;
        }
        
        // Read next chunk
        _file.read((char*)buff, sizeof(buff));
        buff_len = _file.gcount();
    }
    
    auto progress_end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(progress_end - progress_start).count();
    
    // Simple completion summary
    if (total_bytes > 0) {
        std::cout << " ✓ " << (total_bytes / 1024 / 1024) << "MB in " << elapsed << "ms" << std::endl;
    }
}
