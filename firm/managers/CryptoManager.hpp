#ifndef CRYPTO_MANAGER_HPP
#define CRYPTO_MANAGER_HPP

#include <string>
#include <vector>
#include "KeyStorageManager.hpp"
#include "CertificateManager.hpp"
#include "CryptoOperations.hpp"

/**
 * @brief Certificate identifier types.
 */
typedef enum
{
    CA,     /**< CA certificate */
    DEVICE, /**< Device certificate */
    API     /**< API certificate */
} CertificateId;

/**
 * @brief Facade class that manages storage, certificate validation, and cryptographic operations.
 *
 * This class provides a unified interface for:
 * - Storing and retrieving certificates and keys.
 * - Validating certificates (signature verification and key matching).
 * - Performing cryptographic operations such as signing, verifying, encryption, and decryption.
 */
class CryptoManager
{
  public:
    /**
     * @brief Construct a new CryptoManager object.
     */
    CryptoManager();

    /**
     * @brief Stores a certificate.
     *
     * For DEVICE and API certificates, the certificate signature is verified against the stored CA certificate.
     * For a DEVICE certificate, the certificate is also verified to match the stored RSA key.
     *
     * @param id The certificate identifier (CA, DEVICE, or API).
     * @param certData The certificate data as a string.
     * @return "OK" if successful or an error code string if an error occurred.
     */
    std::string SetCertificate(CertificateId id, const std::string &certData);

    /**
     * @brief Retrieves a certificate in hexadecimal format.
     *
     * @param id The certificate identifier (CA, DEVICE, or API).
     * @return The certificate data in hexadecimal format or an error code string if an error occurred.
     */
    std::string GetCertificate(CertificateId id);

    /**
     * @brief Stores an RSA private key.
     *
     * @param keyData The RSA key data as a string.
     * @return "OK" if successful or an error code string if an error occurred.
     */
    std::string SetKeyRSA(const std::string &keyData);

    /**
     * @brief Retrieves the stored RSA key in hexadecimal format.
     *
     * @return The RSA key data in hexadecimal format or an error code string if an error occurred.
     */
    std::string GetKeyRSA();

    /**
     * @brief Removes all stored certificates and keys.
     *
     * @return true if all files were removed successfully, false otherwise.
     */
    bool CleanCrypto();

    /**
     * @brief Returns a semicolon-separated list of stored certificate and key file names.
     *
     * @return A semicolon-separated list of file names.
     */
    std::string GetStoredCertsAndKeys();

    /**
     * @brief Signs data using the stored RSA private key.
     *
     * @param data The data to be signed.
     * @param signature The resulting signature (output).
     * @return true if signing was successful, false otherwise.
     */
    bool SignData(const std::vector<uint8_t> &data, std::vector<uint8_t> &signature);

    /**
     * @brief Verifies a signature using a provided RSA public key.
     *
     * @param publicKeyData The RSA public key data as a string.
     * @param data The data that was signed.
     * @param signature The signature to verify.
     * @return true if the signature is valid, false otherwise.
     */
    bool VerifySignature(const std::string &publicKeyData, const std::vector<uint8_t> &data, const std::vector<uint8_t> &signature);

    /**
     * @brief Encrypts plaintext using a provided RSA public key.
     *
     * @param publicKeyData The RSA public key data as a string.
     * @param plaintext The plaintext data to be encrypted.
     * @param encrypted The resulting encrypted data (output).
     * @return true if encryption was successful, false otherwise.
     */
    bool EncryptWithPublicKey(const std::string &publicKeyData, const std::vector<uint8_t> &plaintext, std::vector<uint8_t> &encrypted);

    /**
     * @brief Decrypts ciphertext using the stored RSA private key.
     *
     * @param encrypted The encrypted data to be decrypted.
     * @param decrypted The resulting decrypted data (output).
     * @return true if decryption was successful, false otherwise.
     */
    bool DecryptWithPrivateKey(const std::vector<uint8_t> &encrypted, std::vector<uint8_t> &decrypted);

  private:
    KeyStorageManager  keyStorageManager;  /**< Manager for storing keys and certificates */
    CertificateManager certificateManager; /**< Manager for certificate validation and matching */
    CryptoOperations   cryptoOperations;   /**< Cryptographic operations handler */
    std::string        mountPoint;         /**< Base mount point for storage files */

    /**
     * @brief Constructs a full file path from a file name.
     *
     * @param fileName The name of the file.
     * @return The full file path as a string.
     */
    std::string GetFilePath(const std::string &fileName);
};

#endif // CRYPTO_MANAGER_HPP
