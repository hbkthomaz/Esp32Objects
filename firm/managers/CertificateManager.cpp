#include "CertificateManager.hpp"
#include <string>
#include <cstdio>
#include <cstring>
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"
#include "mbedtls/error.h"

// Helper function to obtain error messages from mbedtls.
static std::string GetMbedtlsError(int ret)
{
    char errorBuf[128];
    mbedtls_strerror(ret, errorBuf, sizeof(errorBuf));
    return std::string(errorBuf);
}

static std::string NormalizeLineEndings(const std::string &str)
{
    std::string out = str;
    size_t      pos = 0;
    while ((pos = out.find("\r\n", pos)) != std::string::npos)
    {
        out.replace(pos, 2, "\n");
    }
    pos = 0;
    while ((pos = out.find('\r', pos)) != std::string::npos)
    {
        out[pos] = '\n';
    }
    return out;
}

static void TrimString(std::string &s)
{
    const std::string whitespace = " \n\r\t";
    size_t            start      = s.find_first_not_of(whitespace);
    if (start != std::string::npos)
    {
        s = s.substr(start);
    }
    size_t end = s.find_last_not_of(whitespace);
    if (end != std::string::npos)
    {
        s = s.substr(0, end + 1);
    }
}

CertificateManager::CertificateManager()
{
    // Initialization if needed.
}

bool CertificateManager::VerifyCertificateSignature(const std::string &certData, const std::string &caCertData)
{
    std::string certDataForParse = certData;
    if (certData.find("-----BEGIN") != std::string::npos)
    {
        if (certDataForParse.back() != '\0')
        {
            certDataForParse.push_back('\0');
        }
    }

    mbedtls_x509_crt cert;
    mbedtls_x509_crt caCert;
    mbedtls_x509_crt_init(&cert);
    mbedtls_x509_crt_init(&caCert);

    int ret = mbedtls_x509_crt_parse(&cert, reinterpret_cast<const unsigned char *>(certDataForParse.data()), certDataForParse.size());
    if (ret != 0)
    {
        mbedtls_x509_crt_free(&cert);
        mbedtls_x509_crt_free(&caCert);
        return false;
    }

    ret = mbedtls_x509_crt_parse(&caCert, reinterpret_cast<const unsigned char *>(caCertData.data()), caCertData.size());
    if (ret != 0)
    {
        mbedtls_x509_crt_free(&cert);
        mbedtls_x509_crt_free(&caCert);
        return false;
    }

    uint32_t                 flags         = 0;
    mbedtls_x509_crt_profile customProfile = mbedtls_x509_crt_profile_default;
    customProfile.rsa_min_bitlen           = 1024;

    ret = mbedtls_x509_crt_verify_with_profile(&cert, &caCert, nullptr, &customProfile, nullptr, &flags, nullptr, nullptr);

    mbedtls_x509_crt_free(&cert);
    mbedtls_x509_crt_free(&caCert);

    return (ret == 0);
}

bool CertificateManager::ValidateCertKeyMatch(const std::string &certData, const std::string &rsaKeyData)
{
    // Parse the certificate.
    mbedtls_x509_crt cert;
    mbedtls_x509_crt_init(&cert);
    int ret = mbedtls_x509_crt_parse(&cert, reinterpret_cast<const unsigned char *>(certData.data()), certData.size());
    if (ret != 0)
    {
        mbedtls_x509_crt_free(&cert);
        return false;
    }

    // Parse the RSA key.
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    ret = mbedtls_pk_parse_key(&pk, reinterpret_cast<const unsigned char *>(rsaKeyData.data()), rsaKeyData.size(), nullptr, 0, nullptr, nullptr);
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        mbedtls_x509_crt_free(&cert);
        return false;
    }

    char certPubKeyBuffer[2048] = {0};
    ret = mbedtls_pk_write_pubkey_pem(&cert.pk, reinterpret_cast<unsigned char *>(certPubKeyBuffer), sizeof(certPubKeyBuffer));
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        mbedtls_x509_crt_free(&cert);
        return false;
    }

    char keyPubKeyBuffer[2048] = {0};
    ret                        = mbedtls_pk_write_pubkey_pem(&pk, reinterpret_cast<unsigned char *>(keyPubKeyBuffer), sizeof(keyPubKeyBuffer));
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        mbedtls_x509_crt_free(&cert);
        return false;
    }

    bool match = (strcmp(certPubKeyBuffer, keyPubKeyBuffer) == 0);

    mbedtls_pk_free(&pk);
    mbedtls_x509_crt_free(&cert);
    return match;
}
