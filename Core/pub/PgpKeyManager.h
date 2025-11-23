#pragma once

#include <string>
#include <rnp/rnp.h>

/**
 * PgpKeyManager
 *
 * - Owns the rnp_ffi_t context
 * - Generates a simple Ed25519 signing key for a given userid
 * - Exposes:
 *     - ffi()     : rnp_ffi_t context for signing/verifying
 *     - userId()  : userid used for the key (e.g. "demo@example.com")
 * - Optionally can export armored public/secret keys for debugging / inspector.
 */
class PgpKeyManager
{
public:
    /**
     * Construct and generate a new keypair.
     *
     * @param userId   The userid to associate with the key, e.g. "demo@example.com".
     *
     * Throws std::runtime_error on any failure.
     */
    explicit PgpKeyManager(const std::string& userId = "demo@example.com");

    // Clean up rnp state
    ~PgpKeyManager();

    // rnp context (used by RailcardQrGenerator)
    rnp_ffi_t ffi() const { return m_ffi; }

    // The userid we used for the key (also used as signer id)
    const std::string& userId() const { return m_userId; }

    // Optional: export armored keys if you want to show / debug them
    std::string exportPublicKeyArmored() const;
    std::string exportSecretKeyArmored() const;

    // Sign data with the private key and return the signature
    std::string signData(const std::string& data) const;

private:
    rnp_ffi_t   m_ffi = nullptr;
    std::string m_userId;

    void generateKey();                      // Generate key into m_ffi
    rnp_key_handle_t locateUserKey() const;  // Find key by userid
};
