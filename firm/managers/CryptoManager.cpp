#include "CryptoManager.hpp"
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <vector>

CryptoManager::CryptoManager() : mountPoint("/spiffs")
{
    // Initialization if needed.
}

std::string CryptoManager::GetFilePath(const std::string &fileName)
{
    return mountPoint + "/" + fileName;
}

std::string CryptoManager::SetCertificate(CertificateId id, const std::string &certData)
{
    // Validate certificate size.
    if (certData.size() < 100 || certData.size() > 4096)
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

    // For DEVICE and API, verify certificate signature against stored CA certificate.
    if (id == DEVICE || id == API)
    {
        std::string caFilePath = GetFilePath("ca.crt");
        std::string caCertData = keyStorageManager.LoadCertificate(caFilePath);
        if (caCertData.empty())
        {
            return "CA_NOT_LOADED_ERROR";
        }
        if (!certificateManager.VerifyCertificateSignature(certData, caCertData))
        {
            return "CA_SIGNATURE_ERROR";
        }
    }

    // For DEVICE certificate, also verify that certificate matches the stored RSA key.
    if (id == DEVICE)
    {
        std::string keyFilePath = GetFilePath("device.key");
        std::string rsaKeyData  = keyStorageManager.LoadKey(keyFilePath);
        if (rsaKeyData.empty())
        {
            return "RSA_KEY_NOT_LOADED_ERROR";
        }
        if (!certificateManager.ValidateCertKeyMatch(certData, rsaKeyData))
        {
            return "CERT_KEY_MISMATCH_ERROR";
        }
    }

    // Store the certificate.
    std::string filePath = GetFilePath(fileName);
    bool        stored   = keyStorageManager.StoreCertificate(filePath, certData);
    if (!stored)
    {
        return "CERT_FILE_WRITE_ERROR";
    }
    return "OK";
}

std::string CryptoManager::GetCertificate(CertificateId id)
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
    std::string filePath = GetFilePath(fileName);
    std::string certData = keyStorageManager.LoadCertificate(filePath);
    if (certData.empty())
    {
        return "OP_ERROR";
    }
    // Convert certificate to hex representation.
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

std::string CryptoManager::SetKeyRSA(const std::string &keyData)
{
    std::string filePath = GetFilePath("device.key");
    bool        stored   = keyStorageManager.StoreKey(filePath, keyData);
    if (!stored)
    {
        return "KEY_FILE_WRITE_ERROR";
    }
    return "OK";
}

std::string CryptoManager::GetKeyRSA()
{
    std::string filePath = GetFilePath("device.key");
    std::string keyData  = keyStorageManager.LoadKey(filePath);
    if (keyData.empty())
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

bool CryptoManager::CleanCrypto()
{
    bool        allRemoved = true;
    const char *files[]    = {"ca.crt", "device.crt", "api.crt", "device.key"};
    for (const char *fileName : files)
    {
        std::string filePath = GetFilePath(fileName);
        if (!keyStorageManager.RemoveFile(filePath))
        {
            allRemoved = false;
        }
    }
    return allRemoved;
}

std::string CryptoManager::GetStoredCertsAndKeys()
{
    std::string result;
    const char *files[] = {"ca.crt", "device.crt", "api.crt", "device.key"};
    for (const char *fileName : files)
    {
        std::string filePath = GetFilePath(fileName);
        if (keyStorageManager.Exists(filePath))
        {
            if (!result.empty())
            {
                result += ";";
            }
            result += fileName;
        }
    }
    return result;
}

bool CryptoManager::SignData(const std::vector<uint8_t> &data, std::vector<uint8_t> &signature)
{
    // Load the device private key.
    std::string filePath = GetFilePath("device.key");
    std::string keyData  = keyStorageManager.LoadKey(filePath);
    if (keyData.empty())
    {
        return false;
    }
    return cryptoOperations.SignData(keyData, data, signature);
}

bool CryptoManager::VerifySignature(const std::string &publicKeyData, const std::vector<uint8_t> &data, const std::vector<uint8_t> &signature)
{
    return cryptoOperations.VerifySignature(publicKeyData, data, signature);
}

bool CryptoManager::EncryptWithPublicKey(const std::string &publicKeyData, const std::vector<uint8_t> &plaintext, std::vector<uint8_t> &encrypted)
{
    return cryptoOperations.EncryptWithPublicKey(publicKeyData, plaintext, encrypted);
}

bool CryptoManager::DecryptWithPrivateKey(const std::vector<uint8_t> &encrypted, std::vector<uint8_t> &decrypted)
{
    // Load the device private key.
    std::string filePath = GetFilePath("device.key");
    std::string keyData  = keyStorageManager.LoadKey(filePath);
    if (keyData.empty())
    {
        return false;
    }
    return cryptoOperations.DecryptWithPrivateKey(keyData, encrypted, decrypted);
}
