#include "CryptoOperations.hpp"
#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"
#include "mbedtls/error.h"
#include "mbedtls/md.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include <cstring>
#include <vector>
#include <string>

static std::string GetMbedtlsError(int ret)
{
    char errorBuf[128];
    mbedtls_strerror(ret, errorBuf, sizeof(errorBuf));
    return std::string(errorBuf);
}

CryptoOperations::CryptoOperations()
{
    mbedtls_entropy_init(&entropyContext);
    mbedtls_ctr_drbg_init(&ctrDrbgContext);
    const char *pers = "crypto_ops";
    int         ret =
        mbedtls_ctr_drbg_seed(&ctrDrbgContext, mbedtls_entropy_func, &entropyContext, reinterpret_cast<const unsigned char *>(pers), strlen(pers));
    if (ret != 0)
    {
        // Handle error as needed.
    }
}

CryptoOperations::~CryptoOperations()
{
    mbedtls_ctr_drbg_free(&ctrDrbgContext);
    mbedtls_entropy_free(&entropyContext);
}

bool CryptoOperations::SignData(const std::string &privateKeyData, const std::vector<uint8_t> &data, std::vector<uint8_t> &signatureOut)
{
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    int ret = mbedtls_pk_parse_key(&pk, reinterpret_cast<const unsigned char *>(privateKeyData.data()), privateKeyData.size(), nullptr, 0, nullptr,
                                   nullptr);
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    // Compute SHA-256 hash of the input data.
    unsigned char            hash[32] = {0};
    const mbedtls_md_info_t *mdInfo   = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!mdInfo)
    {
        mbedtls_pk_free(&pk);
        return false;
    }
    ret = mbedtls_md(mdInfo, data.data(), data.size(), hash);
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    size_t sigLen = 0;
    signatureOut.resize(512); // Allocate enough space based on key size.
    ret =
        mbedtls_pk_sign(&pk, MBEDTLS_MD_SHA256, hash, 0, signatureOut.data(), signatureOut.size(), &sigLen, mbedtls_ctr_drbg_random, &ctrDrbgContext);
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    signatureOut.resize(sigLen);
    mbedtls_pk_free(&pk);
    return true;
}

bool CryptoOperations::VerifySignature(const std::string &publicKeyData, const std::vector<uint8_t> &data, const std::vector<uint8_t> &signature)
{
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    int ret = mbedtls_pk_parse_public_key(&pk, reinterpret_cast<const unsigned char *>(publicKeyData.data()), publicKeyData.size());
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    unsigned char            hash[32] = {0};
    const mbedtls_md_info_t *mdInfo   = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!mdInfo)
    {
        mbedtls_pk_free(&pk);
        return false;
    }
    ret = mbedtls_md(mdInfo, data.data(), data.size(), hash);
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    ret = mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, 0, signature.data(), signature.size());
    mbedtls_pk_free(&pk);
    return (ret == 0);
}

bool CryptoOperations::EncryptWithPublicKey(const std::string &publicKeyData, const std::vector<uint8_t> &plaintext,
                                            std::vector<uint8_t> &encryptedOut)
{
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    int ret = mbedtls_pk_parse_public_key(&pk, reinterpret_cast<const unsigned char *>(publicKeyData.data()), publicKeyData.size());
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    if (!mbedtls_pk_can_do(&pk, MBEDTLS_PK_RSA))
    {
        mbedtls_pk_free(&pk);
        return false;
    }
    size_t rsaSize = mbedtls_pk_get_len(&pk);
    encryptedOut.resize(rsaSize);

    size_t olen = 0;
    ret = mbedtls_pk_encrypt(&pk, plaintext.data(), plaintext.size(), encryptedOut.data(), &olen, encryptedOut.size(), mbedtls_ctr_drbg_random,
                             &ctrDrbgContext);
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    encryptedOut.resize(olen);
    mbedtls_pk_free(&pk);
    return true;
}

bool CryptoOperations::DecryptWithPrivateKey(const std::string &privateKeyData, const std::vector<uint8_t> &encrypted,
                                             std::vector<uint8_t> &decryptedOut)
{
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    int ret = mbedtls_pk_parse_key(&pk, reinterpret_cast<const unsigned char *>(privateKeyData.data()), privateKeyData.size(), nullptr, 0, nullptr,
                                   nullptr);
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    if (!mbedtls_pk_can_do(&pk, MBEDTLS_PK_RSA))
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    size_t rsaSize = mbedtls_pk_get_len(&pk);
    decryptedOut.resize(rsaSize);

    size_t olen = 0;
    ret = mbedtls_pk_decrypt(&pk, encrypted.data(), encrypted.size(), decryptedOut.data(), &olen, decryptedOut.size(), mbedtls_ctr_drbg_random,
                             &ctrDrbgContext);
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return false;
    }
    decryptedOut.resize(olen);
    mbedtls_pk_free(&pk);
    return true;
}
