#include "PgpKeyManager.h"

#include <stdexcept>
#include <rnp/rnp.h>
#include <rnp/rnp_err.h>

// Constructor: create ffi and generate keypair
PgpKeyManager::PgpKeyManager(const std::string& userId)
    : m_userId(userId)
{
    // Create FFI context (same as in your sample)
    if (rnp_ffi_create(&m_ffi, "GPG", "GPG") != RNP_SUCCESS || !m_ffi) {
        throw std::runtime_error("Failed to create RNP FFI");
    }

    // Generate the keypair into this ffi
    generateKey();
}

// Destructor: destroy ffi
PgpKeyManager::~PgpKeyManager()
{
    if (m_ffi) {
        rnp_ffi_destroy(m_ffi);
        m_ffi = nullptr;
    }
}

void PgpKeyManager::generateKey()
{
    // This mirrors what you had in main(), but without printing.
    // We ask rnp to generate a primary EdDSA signing key for our userid.
    std::string key_json =
        std::string(R"({"primary":{"type":"EDDSA","userid":")") +
        m_userId +
        R"(","usage":["sign"]}})";

    char* gen_results = nullptr;
    if (rnp_generate_key_json(m_ffi, key_json.c_str(), &gen_results) != RNP_SUCCESS ||
        !gen_results) {
        throw std::runtime_error("Key generation failed");
    }

    // We don't actually need to parse the grip for this PoC.
    // The key is now stored in m_ffi and can be found via userid.
    rnp_buffer_destroy(gen_results);
}

// Helper: locate our key by userid (so we can export it)
rnp_key_handle_t PgpKeyManager::locateUserKey() const
{
    rnp_key_handle_t key = nullptr;
    if (rnp_locate_key(m_ffi, "userid", m_userId.c_str(), &key) != RNP_SUCCESS || !key) {
        return nullptr;
    }
    return key;
}

// Export armored public key (optional)
std::string PgpKeyManager::exportPublicKeyArmored() const
{
    rnp_key_handle_t key = locateUserKey();
    if (!key) {
        throw std::runtime_error("Failed to locate user key for public export");
    }

    uint32_t flags_pub = RNP_KEY_EXPORT_ARMORED |
                         RNP_KEY_EXPORT_PUBLIC |
                         RNP_KEY_EXPORT_SUBKEYS;

    rnp_output_t pub_out = nullptr;
    if (rnp_output_to_memory(&pub_out, 0) != RNP_SUCCESS) {
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Failed to create public key output");
    }

    if (rnp_key_export(key, pub_out, flags_pub) != RNP_SUCCESS) {
        rnp_output_destroy(pub_out);
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Public key export failed");
    }

    uint8_t* pub_buf = nullptr;
    size_t   pub_len = 0;
    if (rnp_output_memory_get_buf(pub_out, &pub_buf, &pub_len, false) != RNP_SUCCESS) {
        rnp_output_destroy(pub_out);
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Failed to get public key buffer");
    }

    std::string pubKey(reinterpret_cast<const char*>(pub_buf), pub_len);

    rnp_output_destroy(pub_out);
    rnp_key_handle_destroy(key);

    return pubKey;
}

// Export armored secret key (optional)
std::string PgpKeyManager::exportSecretKeyArmored() const
{
    rnp_key_handle_t key = locateUserKey();
    if (!key) {
        throw std::runtime_error("Failed to locate user key for secret export");
    }

    uint32_t flags_sec = RNP_KEY_EXPORT_ARMORED |
                         RNP_KEY_EXPORT_SECRET |
                         RNP_KEY_EXPORT_SUBKEYS;

    rnp_output_t sec_out = nullptr;
    if (rnp_output_to_memory(&sec_out, 0) != RNP_SUCCESS) {
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Failed to create secret key output");
    }

    if (rnp_key_export(key, sec_out, flags_sec) != RNP_SUCCESS) {
        rnp_output_destroy(sec_out);
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Secret key export failed");
    }

    uint8_t* sec_buf = nullptr;
    size_t   sec_len = 0;
    if (rnp_output_memory_get_buf(sec_out, &sec_buf, &sec_len, false) != RNP_SUCCESS) {
        rnp_output_destroy(sec_out);
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Failed to get secret key buffer");
    }

    std::string secKey(reinterpret_cast<const char*>(sec_buf), sec_len);

    rnp_output_destroy(sec_out);
    rnp_key_handle_destroy(key);

    return secKey;
}

// Sign data with the private key
std::string PgpKeyManager::signData(const std::string& data) const
{
    rnp_key_handle_t key = locateUserKey();
    if (!key) {
        throw std::runtime_error("Failed to locate user key for signing");
    }

    // Create input from data
    rnp_input_t input = nullptr;
    if (rnp_input_from_memory(&input, 
                              reinterpret_cast<const uint8_t*>(data.c_str()), 
                              data.size(), 
                              false) != RNP_SUCCESS) {
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Failed to create input for signing");
    }

    // Create output for signature
    rnp_output_t output = nullptr;
    if (rnp_output_to_memory(&output, 0) != RNP_SUCCESS) {
        rnp_input_destroy(input);
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Failed to create output for signature");
    }

    // Create signing operation
    rnp_op_sign_t sign_op = nullptr;
    if (rnp_op_sign_detached_create(&sign_op, m_ffi, input, output) != RNP_SUCCESS) {
        rnp_output_destroy(output);
        rnp_input_destroy(input);
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Failed to create signing operation");
    }

    // Add signer
    if (rnp_op_sign_add_signature(sign_op, key, nullptr) != RNP_SUCCESS) {
        rnp_op_sign_destroy(sign_op);
        rnp_output_destroy(output);
        rnp_input_destroy(input);
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Failed to add signer");
    }

    // Execute signing
    if (rnp_op_sign_execute(sign_op) != RNP_SUCCESS) {
        rnp_op_sign_destroy(sign_op);
        rnp_output_destroy(output);
        rnp_input_destroy(input);
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Signing operation failed");
    }

    // Get signature
    uint8_t* sig_buf = nullptr;
    size_t sig_len = 0;
    if (rnp_output_memory_get_buf(output, &sig_buf, &sig_len, false) != RNP_SUCCESS) {
        rnp_op_sign_destroy(sign_op);
        rnp_output_destroy(output);
        rnp_input_destroy(input);
        rnp_key_handle_destroy(key);
        throw std::runtime_error("Failed to get signature buffer");
    }

    std::string signature(reinterpret_cast<const char*>(sig_buf), sig_len);

    // Cleanup
    rnp_op_sign_destroy(sign_op);
    rnp_output_destroy(output);
    rnp_input_destroy(input);
    rnp_key_handle_destroy(key);

    return signature;
}
