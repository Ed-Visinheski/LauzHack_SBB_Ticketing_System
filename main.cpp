#include <QApplication>
#include "style.h"
#include "pgpGenerator.h"
#include "window.h"

#include <iostream>
#include <string>
#include <rnp/rnp.h>
#include <rnp/rnp_err.h>

int main() {
    // Currently unused, but kept for future integration.
    Style style;
    PgpGenerator pgpGenerator;
    (void)style;
    (void)pgpGenerator;

    rnp_ffi_t ffi = nullptr;
    if (rnp_ffi_create(&ffi, "GPG", "GPG") != RNP_SUCCESS) {
        std::cerr << "Failed to create RNP FFI\n";
        return 1;
    }

    // Generate a simple Ed25519 signing key with one user id.
    const char* key_json =
        R"({"primary":{"type":"EDDSA","userid":"demo@example.com","usage":["sign"]}})";

    // rnp_generate_key_json needs a JSON output buffer to return key grips.
    char* gen_results = nullptr;
    if (rnp_generate_key_json(ffi, key_json, &gen_results) != RNP_SUCCESS || !gen_results) {
        std::cerr << "Key generation failed\n";
        rnp_ffi_destroy(ffi);
        return 1;
    }

    // Pull the primary key grip out of the JSON output so we can fetch the handle.
    std::string results_str(gen_results);
    rnp_buffer_destroy(gen_results);
    const std::string grip_token = "\"grip\":\"";
    auto grip_pos = results_str.find(grip_token);
    if (grip_pos == std::string::npos) {
        std::cerr << "Key generation succeeded but no grip returned\n";
        rnp_ffi_destroy(ffi);
        return 1;
    }
    grip_pos += grip_token.size();
    const auto end_pos = results_str.find('"', grip_pos);
    if (end_pos == std::string::npos) {
        std::cerr << "Malformed grip in generation results\n";
        rnp_ffi_destroy(ffi);
        return 1;
    }
    const std::string grip = results_str.substr(grip_pos, end_pos - grip_pos);

    rnp_key_handle_t key = nullptr;
    if (rnp_locate_key(ffi, "grip", grip.c_str(), &key) != RNP_SUCCESS || !key) {
        std::cerr << "Failed to locate generated key\n";
        rnp_ffi_destroy(ffi);
        return 1;
    }

    // Export armored public key.
    uint32_t flags_pub = RNP_KEY_EXPORT_ARMORED |
                         RNP_KEY_EXPORT_PUBLIC |
                         RNP_KEY_EXPORT_SUBKEYS;
    rnp_output_t pub_out = nullptr;

    if (rnp_output_to_memory(&pub_out, 0) != RNP_SUCCESS) {
        std::cerr << "Failed to create public key output\n";
        rnp_key_handle_destroy(key);
        rnp_ffi_destroy(ffi);
        return 1;
    }

    if (rnp_key_export(key, pub_out, flags_pub) != RNP_SUCCESS) {
        std::cerr << "Public key export failed\n";
        rnp_output_destroy(pub_out);
        rnp_key_handle_destroy(key);
        rnp_ffi_destroy(ffi);
        return 1;
    }

    uint8_t* pub_buf = nullptr;
    size_t pub_len = 0;
    if (rnp_output_memory_get_buf(pub_out, &pub_buf, &pub_len, false) != RNP_SUCCESS) {
        std::cerr << "Failed to get public key buffer\n";
        rnp_output_destroy(pub_out);
        rnp_key_handle_destroy(key);
        rnp_ffi_destroy(ffi);
        return 1;
    }

    std::cout << "Public key:\n"
              << std::string(reinterpret_cast<const char*>(pub_buf), pub_len) << "\n";

    // Export armored secret key.
    rnp_output_t sec_out = nullptr;
    if (rnp_output_to_memory(&sec_out, 0) != RNP_SUCCESS) {
        std::cerr << "Failed to create secret key output\n";
        rnp_output_destroy(pub_out);
        rnp_key_handle_destroy(key);
        rnp_ffi_destroy(ffi);
        return 1;
    }

    uint32_t flags_sec = RNP_KEY_EXPORT_ARMORED |
                         RNP_KEY_EXPORT_SECRET |
                         RNP_KEY_EXPORT_SUBKEYS;

    if (rnp_key_export(key, sec_out, flags_sec) != RNP_SUCCESS) {
        std::cerr << "Secret key export failed\n";
        rnp_output_destroy(sec_out);
        rnp_output_destroy(pub_out);
        rnp_key_handle_destroy(key);
        rnp_ffi_destroy(ffi);
        return 1;
    }

    uint8_t* sec_buf = nullptr;
    size_t sec_len = 0;
    if (rnp_output_memory_get_buf(sec_out, &sec_buf, &sec_len, false) != RNP_SUCCESS) {
        std::cerr << "Failed to get secret key buffer\n";
        rnp_output_destroy(sec_out);
        rnp_output_destroy(pub_out);
        rnp_key_handle_destroy(key);
        rnp_ffi_destroy(ffi);
        return 1;
    }

    std::cout << "Secret key:\n"
              << std::string(reinterpret_cast<const char*>(sec_buf), sec_len) << "\n";

    // Clean up.
    rnp_output_destroy(pub_out);
    rnp_output_destroy(sec_out);
    rnp_key_handle_destroy(key);
    rnp_ffi_destroy(ffi);

    return 0;
}
