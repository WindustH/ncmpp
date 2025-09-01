
#include "NcmFile.h"
#include "utils.h"
#include "base64.h"
#include "pkcs7.h"
#include "openssl/evp.h"
#include <fstream>
#include <stdexcept>

using namespace std;
using namespace ncm;
using namespace ncm::utils;

namespace {
    const char CORE_HEX_KEY[] = "687A4852416D736F356B496E62617857";
    const char META_HEX_KEY[] = "2331346C6A6B5F215C5D2630553C2728";

    vector<unsigned char> aes_ecb_decrypt(const unsigned char* ciphertext, int ciphertext_len, const unsigned char* key) {
        EVP_CIPHER_CTX* ctx;
        vector<unsigned char> plaintext(ciphertext_len);
        int len;
        int plaintext_len;

        if (!(ctx = EVP_CIPHER_CTX_new())) throw runtime_error("Failed to create new EVP cipher context");

        if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL)) {
            EVP_CIPHER_CTX_free(ctx);
            throw runtime_error("Failed to initialize EVP decryption");
        }

        EVP_CIPHER_CTX_set_padding(ctx, 0);

        if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext, ciphertext_len)) {
            EVP_CIPHER_CTX_free(ctx);
            throw runtime_error("Failed to update EVP decryption");
        }
        plaintext_len = len;

        if (1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len)) {
            EVP_CIPHER_CTX_free(ctx);
            throw runtime_error("Failed to finalize EVP decryption");
        }
        plaintext_len += len;

        EVP_CIPHER_CTX_free(ctx);

        plaintext.resize(plaintext_len);
        return plaintext;
    }
} // anonymous namespace

NcmFile::NcmFile(const filesystem::path& path) : _path(path) {
    _file.open(_path, ios::in | ios::binary);
    if (!_file.is_open()) {
        throw runtime_error("Failed to open file: " + path.string());
    }
}

void NcmFile::dump(const filesystem::path& out_path) {
    _read_key_data();
    _setup_key_box();
    _read_metadata();
    _parse_metadata();
    _dump_audio_data(out_path);
}

void NcmFile::_read_key_data() {
    _file.seekg(10, ios::cur);
    unsigned char key_len_bin[4];
    _file.read((char*)key_len_bin, 4);
    unsigned int key_len = little_int(key_len_bin);

    vector<unsigned char> key_data_bin(key_len);
    _file.read((char*)key_data_bin.data(), key_len);
    for (unsigned int i = 0; i < key_len; i++) {
        key_data_bin[i] ^= 0x64;
    }

    unsigned char core_key[16];
    hex2str(CORE_HEX_KEY, core_key);
    vector<unsigned char> decrypted_key = aes_ecb_decrypt(key_data_bin.data(), key_len, core_key);

    // Unpad the decrypted key
    unsigned int unpadded_len = pkcs7::pad_size(decrypted_key.data(), decrypted_key.size());
    _key_data.resize(unpadded_len);
    pkcs7::unpad(decrypted_key.data(), decrypted_key.size(), _key_data.data());
}

void NcmFile::_setup_key_box() {
    unsigned char* key_data_use = _key_data.data() + 17;
    unsigned int key_len_unpad = _key_data.size();

    _key_box.resize(256);
    for (unsigned int i = 0; i < 256; i++) {
        _key_box[i] = i;
    }

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

void NcmFile::_dump_audio_data(const filesystem::path& out_path) {
    _file.seekg(9, ios::cur);
    unsigned char image_len_bin[4];
    _file.read((char*)image_len_bin, 4);
    unsigned int image_len = little_int(image_len_bin);

    if (image_len > 0) {
        vector<unsigned char> image_data(image_len);
        _file.read((char*)image_data.data(), image_len);

        filesystem::path cover_path = out_path;
        cover_path.replace_extension("jpg");

        if (cover_path.has_parent_path()) {
            filesystem::create_directories(cover_path.parent_path());
        }

        ofstream cover_of(cover_path, ios::out | ios::binary);
        if (cover_of.is_open()) {
            cover_of.write((char*)image_data.data(), image_len);
            cover_of.close();
        }
    } else {
        _file.seekg(image_len, ios::cur);
    }

    string extname = "." + string(_metadata["format"].GetString());
    filesystem::path tgt = out_path;
    tgt.replace_extension(extname);

    if (tgt.has_parent_path()) {
        filesystem::create_directories(tgt.parent_path());
    }

    ofstream of(tgt, ios::out | ios::binary);
    if (!of.is_open()) {
        throw runtime_error("Failed to open output file: " + tgt.string());
    }

    unsigned char buff[0x8000];
    _file.read((char*)buff, sizeof(buff));
    unsigned int buff_len = _file.gcount();
    while (buff_len > 0) {
        for (unsigned int i = 0; i < buff_len; i++) {
            int j = (i + 1) & 0xff;
            buff[i] ^= _key_box[(_key_box[j] + _key_box[(_key_box[j] + j) & 0xff]) & 0xff];
        }
        of.write((char*)buff, buff_len);
        _file.read((char*)buff, sizeof(buff));
        buff_len = _file.gcount();
    }
}
