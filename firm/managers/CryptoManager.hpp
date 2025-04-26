#ifndef CRYPTO_MANAGER_HPP
#define CRYPTO_MANAGER_HPP

#include <string>
#include <vector>
#include "CryptoStorageManager.hpp"
#include "CertificateManager.hpp"
#include "CryptoOperations.hpp"

/**
 * @brief Enumeration for certificate identifier types.
 */
enum CertificateId
{
    CA,     /**< CA certificate */
    DEVICE, /**< Device certificate */
    API     /**< API certificate */
};

/**
 * @brief Facade class for managing certificates, keys, and cryptographic operations.
 *
 * This class provides methods for:
 * - Storing and retrieving certificates and RSA keys in persistent storage.
 * - Validating certificates (e.g., signature verification and key matching).
 * - Performing cryptographic operations such as signing, verifying, encryption, and decryption.
 * - Configuring the RSA key size for new key generation (allowed only if no certificate/key exists).
 */
class CryptoManager
{
  public:
    /**
     * @brief Constructs a new CryptoManager object.
     *
     * Initializes the mount point and loads the RSA key size from persistent storage.
     */
    CryptoManager();

    /**
     * @brief Initializes the CryptoManager.
     */
    void Init(void);

    /**
     * @brief Stores a certificate.
     *
     * For DEVICE and API certificates, the certificate signature is verified against the stored CA certificate.
     * For a DEVICE certificate, the certificate is also checked against the stored RSA key.
     *
     * @param id The certificate identifier (CA, DEVICE, or API).
     * @param certData The certificate data as a string.
     * @return "OK" if successful, or an error code string if an error occurred.
     */
    std::string SetCertificate(CertificateId id, const std::string &certData);

    /**
     * @brief Retrieves a certificate in hexadecimal format.
     *
     * @param id The certificate identifier (CA, DEVICE, or API).
     * @return The certificate data as a hex string or an error code string if an error occurred.
     */
    std::string GetCertificate(CertificateId id);

    /**
     * @brief Stores the device's RSA private key.
     *
     * @param keyData The RSA key data as a string.
     * @return "OK" if successful, or an error code string if an error occurred.
     */
    std::string SetKeyRSA(const std::string &keyData);

    /**
     * @brief Retrieves the stored RSA key in hexadecimal format.
     *
     * @return The RSA key data as a hex string or an error code string if an error occurred.
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
     * @return A semicolon-separated string listing the file names.
     */
    std::string GetStoredCertsAndKeys();

    /**
     * @brief Signs data using the stored RSA private key.
     *
     * @param data The data to sign.
     * @param signature The resulting signature (output).
     * @return true if signing was successful, false otherwise.
     */
    bool SignData(const std::vector<uint8_t> &data, std::vector<uint8_t> &signature);

    /**
     * @brief Verifies a signature using the provided RSA public key.
     *
     * @param publicKeyData The RSA public key data as a string.
     * @param data The data that was signed.
     * @param signature The signature to verify.
     * @return true if the signature is valid, false otherwise.
     */
    bool VerifySignature(const std::string &publicKeyData, const std::vector<uint8_t> &data, const std::vector<uint8_t> &signature);

    /**
     * @brief Encrypts plaintext using the provided RSA public key.
     *
     * @param publicKeyData The RSA public key data as a string.
     * @param plaintext The plaintext data to encrypt.
     * @param encrypted The resulting encrypted data (output).
     * @return true if encryption was successful, false otherwise.
     */
    bool EncryptWithPublicKey(const std::string &publicKeyData, const std::vector<uint8_t> &plaintext, std::vector<uint8_t> &encrypted);

    /**
     * @brief Decrypts ciphertext using the stored RSA private key.
     *
     * @param encrypted The encrypted data to decrypt.
     * @param decrypted The resulting decrypted data (output).
     * @return true if decryption was successful, false otherwise.
     */
    bool DecryptWithPrivateKey(const std::vector<uint8_t> &encrypted, std::vector<uint8_t> &decrypted);

    /**
     * @brief Checks whether it is permitted to change the RSA key size.
     *
     * The RSA key size can only be changed if neither the CA certificate nor the device key is stored.
     *
     * @return true if key size can be changed, false otherwise.
     */
    bool CanChangeKeySize() const;

    /**
     * @brief Sets the RSA key size for future key generation.
     *
     * This operation is allowed only if no CA certificate and no device key have been stored.
     *
     * @param newKeySize The desired RSA key size (e.g., 1024 or 2048).
     * @return true if the key size was updated successfully, false otherwise.
     */
    bool SetRsaKeySize(uint16_t newKeySize);

  private:
    CryptoStorageManager cryptoStorageManager; /**< Manager for storing keys and certificates */
    CertificateManager   certificateManager;   /**< Manager for certificate validation and matching */
    CryptoOperations     cryptoOperations;     /**< Cryptographic operations handler */
    std::string          mountPoint;           /**< Base mount point for storage files */
    uint16_t             rsaKeySize;           /**< Configured RSA key size (default is 1024) */

    /**
     * @brief Constructs a full file path from a file name.
     *
     * @param fileName The name of the file.
     * @return The full file path as a string.
     */
    std::string GetFilePath(const std::string &fileName) const;

    /**
     * @brief Loads the RSA key size from persistent storage.
     *
     * If the value is invalid or missing, defaults to 1024.
     */
    void LoadRsaKeySizeFromStorage();

    /**
     * @brief Saves the current RSA key size to persistent storage.
     *
     * @return true if saved successfully, false otherwise.
     */
    bool SaveRsaKeySizeToStorage();
};

#endif // CRYPTO_MANAGER_HPP
