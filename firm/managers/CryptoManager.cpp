#include "CryptoManager.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <inttypes.h>

#include "mbedtls/error.h"
#include "mbedtls/base64.h"
#include "mbedtls/bignum.h"

extern "C"
{
#include "esp_err.h"
#include "esp_spiffs.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"
}

static std::string GetMbedtlsError(int ret)
{
    char errorBuf[128];
    mbedtls_strerror(ret, errorBuf, sizeof(errorBuf));
    return std::string(errorBuf);
}

static constexpr const char *mountPoint = "/spiffs";

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

static void PrintRsaKeyModulus(const std::string &rsaKeyData)
{
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    int ret = mbedtls_pk_parse_key(&pk, reinterpret_cast<const unsigned char *>(rsaKeyData.data()), rsaKeyData.size(), nullptr, 0, nullptr, nullptr);

    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return;
    }

    if (mbedtls_pk_get_type(&pk) != MBEDTLS_PK_RSA)
    {
        mbedtls_pk_free(&pk);
        return;
    }

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
    mbedtls_mpi          N;
    mbedtls_mpi_init(&N);

    ret = mbedtls_rsa_export(rsa, &N, nullptr, nullptr, nullptr, nullptr);
    if (ret != 0)
    {
        mbedtls_mpi_free(&N);
        mbedtls_pk_free(&pk);
        return;
    }

    char   buffer[1024] = {0};
    size_t olen         = 0;
    ret                 = mbedtls_mpi_write_string(&N, 16, buffer, sizeof(buffer), &olen);

    mbedtls_mpi_free(&N);
    mbedtls_pk_free(&pk);
}

static void PrintCertPublicKeyModulus(const std::string &certData)
{
    mbedtls_x509_crt cert;
    mbedtls_x509_crt_init(&cert);

    int ret = mbedtls_x509_crt_parse(&cert, reinterpret_cast<const unsigned char *>(certData.data()), certData.size());

    if (ret != 0)
    {
        mbedtls_x509_crt_free(&cert);
        return;
    }

    if (mbedtls_pk_get_type(&cert.pk) != MBEDTLS_PK_RSA)
    {
        mbedtls_x509_crt_free(&cert);
        return;
    }

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa(cert.pk);
    mbedtls_mpi          N;
    mbedtls_mpi_init(&N);

    ret = mbedtls_rsa_export(rsa, &N, nullptr, nullptr, nullptr, nullptr);
    if (ret != 0)
    {
        mbedtls_mpi_free(&N);
        mbedtls_x509_crt_free(&cert);
        return;
    }

    char   buffer[1024] = {0};
    size_t olen         = 0;
    ret                 = mbedtls_mpi_write_string(&N, 16, buffer, sizeof(buffer), &olen);

    mbedtls_mpi_free(&N);
    mbedtls_x509_crt_free(&cert);
}

CryptoManager::CryptoManager()
{
}

bool CryptoManager::ValidateCertificateSize(const std::string &certificateData)
{
    size_t certSize = certificateData.size();
    return (certSize >= 100 && certSize <= 4096);
}

bool CryptoManager::VerifyCertificateSignature(const std::string &certData, const std::string &caCertData)
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

bool CryptoManager::ValidateCertKeyMatch(const std::string &certData, const std::string &rsaKeyData)
{
    if (!VerifyRsaKeyFormat(rsaKeyData))
    {
        return false;
    }

    PrintRsaKeyModulus(rsaKeyData);
    PrintCertPublicKeyModulus(certData);

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

std::string CryptoManager::CertificateSet(certificateId_et id, const std::string &buffer, uint32_t size)
{
    int ret = 0;

    if (!ValidateCertificateSize(buffer))
    {
        return "INVALID_CERTIFICATE_SIZE";
    }

    std::string fileName;
    switch (id)
    {
        case CA:
            fileName = "ca.crt";
            break;
        case DEVICE:
            fileName = "device.crt";
            break;
        case API:
            fileName = "api.crt";
            break;
        default:
            return "INVALID_CERTIFICATE_ID";
    }

    if (id == API || id == DEVICE)
    {
        std::string caFilePath = std::string(mountPoint) + "/ca.crt";
        FILE       *caFile     = fopen(caFilePath.c_str(), "rb");
        if (!caFile)
        {
            return "CA_NOT_LOADED_ERROR";
        }
        fseek(caFile, 0, SEEK_END);
        long caSize = ftell(caFile);
        fseek(caFile, 0, SEEK_SET);

        std::string caCertData;
        caCertData.resize(caSize);

        size_t bytesRead = fread(&caCertData[0], 1, caSize, caFile);
        fclose(caFile);

        if (bytesRead != static_cast<size_t>(caSize))
        {
            return "CA_FILE_READ_ERROR";
        }

        if (!VerifyCertificateSignature(buffer, caCertData))
        {
            return "CA_SIGNATURE_ERROR";
        }
    }

    if (id == DEVICE)
    {
        std::string keyFilePath = std::string(mountPoint) + "/device.key";
        FILE       *keyFile     = fopen(keyFilePath.c_str(), "rb");
        if (!keyFile)
        {
            return "RSA_KEY_NOT_LOADED_ERROR";
        }
        fseek(keyFile, 0, SEEK_END);
        long keySize = ftell(keyFile);
        fseek(keyFile, 0, SEEK_SET);

        std::string rsaKeyData;
        rsaKeyData.resize(keySize);

        size_t keyBytesRead = fread(&rsaKeyData[0], 1, keySize, keyFile);
        fclose(keyFile);

        if (keyBytesRead != static_cast<size_t>(keySize))
        {
            return "DEVICE_KEY_READ_ERROR";
        }

        if (!ValidateCertKeyMatch(buffer, rsaKeyData))
        {
            return "CERT_KEY_MISMATCH_ERROR";
        }
    }

    std::string filePath = std::string(mountPoint) + "/" + fileName;
    FILE       *f        = fopen(filePath.c_str(), "wb");
    if (!f)
    {
        return "CERT_FILE_OPEN_ERROR";
    }

    size_t written = fwrite(buffer.data(), 1, size, f);
    fclose(f);
    if (written != size)
    {
        return "CERT_FILE_WRITE_ERROR";
    }

    FILE *f2 = fopen(filePath.c_str(), "rb");
    if (!f2)
    {
        return "CERT_FILE_READ_OPEN_ERROR";
    }
    fseek(f2, 0, SEEK_END);
    long fileSize = ftell(f2);
    fseek(f2, 0, SEEK_SET);

    std::string fileCertData;
    fileCertData.resize(fileSize);

    size_t readBytes = fread(&fileCertData[0], 1, fileSize, f2);
    fclose(f2);

    if (readBytes != static_cast<size_t>(fileSize))
    {
        return "CERT_FILE_READ_ERROR";
    }

    {
        std::string hexDebug;
        for (size_t i = 0; i < 16 && i < fileCertData.size(); i++)
        {
            char buf[4];
            snprintf(buf, sizeof(buf), "%02X ", static_cast<unsigned char>(fileCertData[i]));
            hexDebug += buf;
        }
    }

    std::string certDataToParse;
    bool        isPem = (fileCertData.find("-----BEGIN") != std::string::npos);
    if (isPem)
    {
        certDataToParse = NormalizeLineEndings(fileCertData);
        TrimString(certDataToParse);
        if (certDataToParse.back() != '\0')
        {
            certDataToParse.push_back('\0');
        }
    }
    else
    {
        certDataToParse = fileCertData;
    }

    mbedtls_x509_crt cert;
    mbedtls_x509_crt_init(&cert);

    ret = mbedtls_x509_crt_parse(&cert, reinterpret_cast<const unsigned char *>(certDataToParse.data()), certDataToParse.size());
    if (ret != 0)
    {
        mbedtls_x509_crt_free(&cert);
        return std::string("CERT_PARSE_ERROR: ") + GetMbedtlsError(ret);
    }

    char pubKeyBuffer[2048] = {0};
    ret                     = mbedtls_pk_write_pubkey_pem(&cert.pk, reinterpret_cast<unsigned char *>(pubKeyBuffer), sizeof(pubKeyBuffer));
    if (ret != 0)
    {
        mbedtls_x509_crt_free(&cert);
        return std::string("PUBKEY_WRITE_ERROR: ") + GetMbedtlsError(ret);
    }

    std::string publicKey(pubKeyBuffer);
    mbedtls_x509_crt_free(&cert);
    return "OK";
}

std::string CryptoManager::KeySetRSA(const std::string &buffer, uint32_t size)
{
    std::string fileName = "device.key";
    std::string filePath = std::string(mountPoint) + "/" + fileName;

    FILE *f = fopen(filePath.c_str(), "wb");
    if (!f)
    {
        return "KEY_FILE_OPEN_ERROR";
    }
    size_t written = fwrite(buffer.data(), 1, size, f);
    fclose(f);

    if (written != size)
    {
        return "KEY_FILE_WRITE_ERROR";
    }

    return "OK";
}

std::string CryptoManager::KeyGetRSA()
{
    std::string fileName = "device.key";
    std::string filePath = std::string(mountPoint) + "/" + fileName;

    FILE *file = fopen(filePath.c_str(), "rb");
    if (!file)
    {
        return "OP_ERROR";
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::string keyData;
    keyData.resize(fileSize);

    size_t bytesRead = fread(&keyData[0], 1, fileSize, file);
    fclose(file);

    if (bytesRead != static_cast<size_t>(fileSize))
    {
        return "OP_ERROR";
    }

    std::string hexDisplay;
    hexDisplay.reserve(keyData.size() * 2);

    for (unsigned char ch : keyData)
    {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02X", ch);
        hexDisplay.append(buf);
    }

    return hexDisplay;
}

std::string CryptoManager::CertificateGet(certificateId_et id)
{
    std::string fileName;
    switch (id)
    {
        case CA:
            fileName = "ca.crt";
            break;
        case DEVICE:
            fileName = "device.crt";
            break;
        case API:
            fileName = "api.crt";
            break;
        default:
            return "SYNTAX_ERROR";
    }

    std::string filePath = std::string(mountPoint) + "/" + fileName;
    FILE       *file     = fopen(filePath.c_str(), "rb");
    if (!file)
    {
        return "OP_ERROR";
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::string certData;
    certData.resize(fileSize);

    size_t bytesRead = fread(&certData[0], 1, fileSize, file);
    fclose(file);

    if (bytesRead != static_cast<size_t>(fileSize))
    {
        return "OP_ERROR";
    }

    std::string hexDisplay;
    hexDisplay.reserve(certData.size() * 2);

    for (unsigned char ch : certData)
    {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02X", ch);
        hexDisplay.append(buf);
    }

    return hexDisplay;
}

bool CryptoManager::CleanCrypto()
{
    std::vector<std::string> filesToRemove = {"ca.crt", "device.crt", "api.crt", "device.key"};

    bool allRemoved = true;

    for (const auto &fileName : filesToRemove)
    {
        std::string filePath = std::string(mountPoint) + "/" + fileName;
        if (remove(filePath.c_str()) != 0)
        {
            if (errno != ENOENT)
            {
                allRemoved = false;
            }
        }
    }

    return allRemoved;
}

std::string CryptoManager::GetStoredCertsAndKeys()
{
    std::vector<std::string> possibleFiles = {"ca.crt", "device.crt", "api.crt", "device.key"};
    std::string              result;

    for (const auto &fileName : possibleFiles)
    {
        std::string filePath = std::string(mountPoint) + "/" + fileName;
        FILE       *file     = fopen(filePath.c_str(), "rb");
        if (file)
        {
            fclose(file);
            if (!result.empty())
            {
                result += ";";
            }
            result += fileName;
        }
    }
    return result;
}

bool CryptoManager::VerifyRsaKeyFormat(const std::string &rsaKeyData)
{
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);

    int ret = mbedtls_pk_parse_key(&pk, reinterpret_cast<const unsigned char *>(rsaKeyData.data()), rsaKeyData.size(), nullptr, 0, nullptr, nullptr);

    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    if (mbedtls_pk_get_type(&pk) != MBEDTLS_PK_RSA)
    {
        mbedtls_pk_free(&pk);
        return false;
    }

    mbedtls_pk_free(&pk);
    return true;
}
