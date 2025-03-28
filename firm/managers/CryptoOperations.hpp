#ifndef CRYPTO_OPERATIONS_HPP
#define CRYPTO_OPERATIONS_HPP

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"

/**
 * @brief Provides cryptographic operations such as signing, verifying, encryption, and decryption.
 *
 * This class uses mbedtls to implement RSA-based cryptographic operations including:
 * - Signing data with an RSA private key.
 * - Verifying a signature with an RSA public key.
 * - Encrypting data with an RSA public key.
 * - Decrypting data with an RSA private key.
 */
class CryptoOperations
{
  public:
    /**
     * @brief Construct a new CryptoOperations object.
     *
     * Initializes the entropy and CTR-DRBG contexts required for cryptographic operations.
     */
    CryptoOperations();

    /**
     * @brief Destroy the CryptoOperations object.
     *
     * Frees the mbedtls contexts used for cryptographic operations.
     */
    ~CryptoOperations();

    /**
     * @brief Signs data using an RSA private key.
     *
     * This function computes the SHA-256 hash of the input data and signs it using the provided RSA private key.
     *
     * @param privateKeyData The RSA private key data as a string.
     * @param data The input data to be signed.
     * @param signatureOut The resulting signature (output).
     * @return true if the signing operation was successful, false otherwise.
     */
    bool SignData(const std::string &privateKeyData, const std::vector<uint8_t> &data, std::vector<uint8_t> &signatureOut);

    /**
     * @brief Verifies a signature using an RSA public key.
     *
     * This function computes the SHA-256 hash of the input data and verifies the provided signature
     * against it using the RSA public key.
     *
     * @param publicKeyData The RSA public key data as a string.
     * @param data The data that was signed.
     * @param signature The signature to verify.
     * @return true if the signature is valid, false otherwise.
     */
    bool VerifySignature(const std::string &publicKeyData, const std::vector<uint8_t> &data, const std::vector<uint8_t> &signature);

    /**
     * @brief Encrypts plaintext using an RSA public key.
     *
     * Encrypts the provided plaintext with the RSA public key.
     *
     * @param publicKeyData The RSA public key data as a string.
     * @param plaintext The plaintext data to be encrypted.
     * @param encryptedOut The resulting encrypted data (output).
     * @return true if encryption was successful, false otherwise.
     */
    bool EncryptWithPublicKey(const std::string &publicKeyData, const std::vector<uint8_t> &plaintext, std::vector<uint8_t> &encryptedOut);

    /**
     * @brief Decrypts ciphertext using an RSA private key.
     *
     * Decrypts the provided ciphertext with the RSA private key.
     *
     * @param privateKeyData The RSA private key data as a string.
     * @param encrypted The encrypted data to be decrypted.
     * @param decryptedOut The resulting decrypted data (output).
     * @return true if decryption was successful, false otherwise.
     */
    bool DecryptWithPrivateKey(const std::string &privateKeyData, const std::vector<uint8_t> &encrypted, std::vector<uint8_t> &decryptedOut);

  private:
    mbedtls_entropy_context  entropyContext; /**< mbedtls entropy context */
    mbedtls_ctr_drbg_context ctrDrbgContext; /**< mbedtls CTR-DRBG context */
};

#endif // CRYPTO_OPERATIONS_HPP
