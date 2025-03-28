#ifndef CERTIFICATE_MANAGER_HPP
#define CERTIFICATE_MANAGER_HPP

#include <string>

/**
 * @brief Provides certificate validation and matching functionalities.
 *
 * This class offers methods to verify certificate signatures and to validate whether a certificate
 * matches an RSA key.
 */
class CertificateManager
{
  public:
    /**
     * @brief Construct a new CertificateManager object.
     */
    CertificateManager();

    /**
     * @brief Verifies the signature of a certificate using a CA certificate.
     *
     * The function parses both the certificate and the CA certificate, then verifies the certificate's
     * signature against the provided CA certificate.
     *
     * @param certData The certificate data as a string.
     * @param caCertData The CA certificate data as a string.
     * @return true if the certificate signature is valid, false otherwise.
     */
    bool VerifyCertificateSignature(const std::string &certData, const std::string &caCertData);

    /**
     * @brief Validates that the certificate matches the given RSA key.
     *
     * This function checks if the public key extracted from the certificate matches the public key
     * derived from the RSA key.
     *
     * @param certData The certificate data as a string.
     * @param rsaKeyData The RSA key data as a string.
     * @return true if the certificate and key match, false otherwise.
     */
    bool ValidateCertKeyMatch(const std::string &certData, const std::string &rsaKeyData);
};

#endif // CERTIFICATE_MANAGER_HPP
