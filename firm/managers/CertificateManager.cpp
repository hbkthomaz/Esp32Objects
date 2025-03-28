#include "CertificateManager.hpp"
#include <string>
#include <cstdio>
#include <cstring>
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"
#include "mbedtls/error.h"

CertificateManager::CertificateManager()
{
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
    mbedtls_x509_crt cert;
    mbedtls_x509_crt_init(&cert);
    int ret = mbedtls_x509_crt_parse(&cert, reinterpret_cast<const unsigned char *>(certData.data()), certData.size());
    if (ret != 0)
    {
        mbedtls_x509_crt_free(&cert);
        return false;
    }

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

int CertificateManager::GetRsaKeySizeFromCertificate(const std::string &certData)
{
    mbedtls_x509_crt cert;
    mbedtls_x509_crt_init(&cert);
    std::string certDataForParse = certData;
    if (certData.find("-----BEGIN") != std::string::npos)
    {
        if (certDataForParse.back() != '\0')
        {
            certDataForParse.push_back('\0');
        }
    }
    int ret = mbedtls_x509_crt_parse(&cert, reinterpret_cast<const unsigned char *>(certDataForParse.data()), certDataForParse.size());
    if (ret != 0)
    {
        mbedtls_x509_crt_free(&cert);
        return -1;
    }
    if (mbedtls_pk_get_type(&cert.pk) != MBEDTLS_PK_RSA)
    {
        mbedtls_x509_crt_free(&cert);
        return -1;
    }
    mbedtls_rsa_context *rsa     = mbedtls_pk_rsa(cert.pk);
    int                  keySize = static_cast<int>(mbedtls_rsa_get_len(rsa) * 8);
    mbedtls_x509_crt_free(&cert);
    return keySize;
}

int CertificateManager::GetRsaKeySizeFromKey(const std::string &keyData)
{
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    std::string keyDataForParse = keyData;
    if (keyData.find("-----BEGIN") != std::string::npos)
    {
        if (keyDataForParse.back() != '\0')
        {
            keyDataForParse.push_back('\0');
        }
    }
    int ret = mbedtls_pk_parse_key(&pk, reinterpret_cast<const unsigned char *>(keyDataForParse.data()), keyDataForParse.size(), nullptr, 0, nullptr,
                                   nullptr);
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return -1;
    }
    if (mbedtls_pk_get_type(&pk) != MBEDTLS_PK_RSA)
    {
        mbedtls_pk_free(&pk);
        return -1;
    }
    mbedtls_rsa_context *rsa     = mbedtls_pk_rsa(pk);
    int                  keySize = static_cast<int>(mbedtls_rsa_get_len(rsa) * 8);
    mbedtls_pk_free(&pk);
    return keySize;
}
