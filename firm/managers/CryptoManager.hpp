#ifndef CRYPTO_MANAGER_HPP
#define CRYPTO_MANAGER_HPP

#include <string>
#include <cstdint>

/**
 * @brief Certificate identifier types.
 */
typedef enum
{
    CA,     ///< CA certificate
    DEVICE, ///< Device certificate
    API     ///< API certificate
} certificateId_et;

/**
 * @brief Manages certificates and RSA keys in SPIFFS.
 */
class CryptoManager
{
  public:
    /**
     * @brief Default constructor.
     */
    CryptoManager();

    /**
     * @brief Stores a certificate (CA, DEVICE, or API).
     *
     * - CA is stored without verification.
     * - DEVICE or API is verified against a previously stored CA.
     * - DEVICE is also checked against the stored RSA key.
     *
     * @param id Certificate type (CA, DEVICE, or API).
     * @param buffer Certificate content.
     * @param size Size of the certificate data.
     * @return "OK" or an error code.
     */
    std::string CertificateSet(certificateId_et id, const std::string &buffer, uint32_t size);

    /**
     * @brief Retrieves a certificate in hex format.
     *
     * @param id Certificate type (CA, DEVICE, or API).
     * @return Certificate as hex or error string.
     */
    std::string CertificateGet(certificateId_et id);

    /**
     * @brief Stores an RSA private key.
     *
     * @param buffer Key content.
     * @param size Size of the key data.
     * @return "OK" or an error code.
     */
    std::string KeySetRSA(const std::string &buffer, uint32_t size);

    /**
     * @brief Retrieves the stored RSA key in hex format.
     *
     * @return RSA key as hex or error string.
     */
    std::string KeyGetRSA();

    /**
     * @brief Removes all stored certificates and keys.
     *
     * @return True if all removed, false otherwise.
     */
    bool CleanCrypto(void);

    /**
     * @brief Lists stored certificates and keys.
     *
     * @return Filenames in a semicolon-separated list or empty if none.
     */
    std::string GetStoredCertsAndKeys();

  private:
    /**
     * @brief Checks if certificate size is valid (100 to 4096 bytes).
     */
    bool ValidateCertificateSize(const std::string &certificateData);

    /**
     * @brief Verifies a certificate signature against a CA certificate.
     */
    bool VerifyCertificateSignature(const std::string &certData, const std::string &caCertData);

    /**
     * @brief Checks if a device certificate matches the stored RSA key.
     */
    bool ValidateCertKeyMatch(const std::string &certData, const std::string &rsaKeyData);

    /**
     * @brief Verifies RSA key format (PEM/DER).
     */
    bool VerifyRsaKeyFormat(const std::string &rsaKeyData);
};

#endif // CRYPTO_MANAGER_HPP
