#include "CommandManager.hpp"
#include "FlashManager.hpp"
#include "CryptoManager.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdint>
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/error.h"

static FlashManager  flashManager;
static CryptoManager cryptoManager;

static std::string GetAPIPublicKey()
{
    const std::string filePath = "/spiffs/api.crt";
    FILE             *file     = fopen(filePath.c_str(), "rb");
    if (!file)
    {
        return "OP_ERROR";
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    std::string certData;
    certData.resize(fileSize);
    size_t readBytes = fread(&certData[0], 1, fileSize, file);
    fclose(file);
    if (readBytes != static_cast<size_t>(fileSize))
    {
        return "OP_ERROR";
    }

    if (certData.find("-----BEGIN") != std::string::npos)
    {
        if (certData.back() != '\0')
        {
            certData.push_back('\0');
        }
    }

    mbedtls_x509_crt cert;
    mbedtls_x509_crt_init(&cert);
    int ret = mbedtls_x509_crt_parse(&cert, reinterpret_cast<const unsigned char *>(certData.data()), certData.size());
    if (ret != 0)
    {
        mbedtls_x509_crt_free(&cert);
        return "OP_ERROR";
    }

    char pubKeyBuffer[2048] = {0};
    ret                     = mbedtls_pk_write_pubkey_pem(&cert.pk, reinterpret_cast<unsigned char *>(pubKeyBuffer), sizeof(pubKeyBuffer));
    mbedtls_x509_crt_free(&cert);
    if (ret != 0)
    {
        return "OP_ERROR";
    }
    return std::string(pubKeyBuffer);
}

CommandManager::CommandManager()
{
}

void CommandManager::Init()
{
    flashManager.Init();
    cryptoManager.Init();
}

std::string CommandManager::ProcessCommand(const std::string &cmdOriginal)
{
    std::string cmd  = cmdOriginal;
    auto        trim = [](std::string &s)
    {
        while (!s.empty() && (s.front() == ' ' || s.front() == '\n' || s.front() == '\r' || s.front() == '\t'))
        {
            s.erase(s.begin());
        }
        while (!s.empty() && (s.back() == ' ' || s.back() == '\n' || s.back() == '\r' || s.back() == '\t'))
        {
            s.pop_back();
        }
    };
    trim(cmd);

    if (cmd.empty())
    {
        return "SYNTAX_ERROR";
    }
    else if ((cmd[0] == 't') && (cmd.size() > 1))
    {
        return CommandTests(cmd.substr(1));
    }
    else if ((cmd[0] == 'c') && (cmd.size() > 1))
    {
        return CommandCryptoSet(cmd.substr(1));
    }
    else if ((cmd[0] == 'g') && (cmd.size() > 1))
    {
        return CommandCryptoGet(cmd.substr(1));
    }
    else if ((cmd[0] == 'o') && (cmd.size() > 1))
    {
        return CommandCryptoOpen(cmd.substr(1));
    }
    else if ((cmd[0] == 'r') && (cmd.size() > 1))
    {
        return CommandRsaKeySize(cmd.substr(1));
    }

    return "SYNTAX_ERROR";
}

std::string CommandManager::CommandTests(const std::string &cmd)
{
    char        subCmd       = cmd[0];
    std::string remainingCmd = cmd.substr(1);
    if (subCmd == 'f')
    {
        return flashManager.HandleCommand(remainingCmd);
    }
    return "SYNTAX_ERROR";
}

std::string CommandManager::CommandCryptoGet(const std::string &cmd)
{
    if ((cmd[0] == 'c') && (cmd.size() >= 2))
    {
        char          certIdChar = cmd[1];
        int           certIdInt  = static_cast<int>(certIdChar - '0');
        CertificateId certId     = static_cast<CertificateId>(certIdInt);
        return cryptoManager.GetCertificate(certId);
    }
    else if ((cmd[0] == 'k') && (cmd.size() >= 2))
    {
        return cryptoManager.GetKeyRSA();
    }
    return "SYNTAX_ERROR";
}

std::string CommandManager::CommandCryptoSet(const std::string &cmd)
{
    if ((cmd[0] == 'c') && (cmd.size() > 2))
    {
        char          certIdChar = cmd[1];
        int           certIdInt  = static_cast<int>(certIdChar - '0');
        CertificateId certId     = static_cast<CertificateId>(certIdInt);
        std::string   hexBuffer  = cmd.substr(2);
        std::string   binBuffer  = HexToBytes(hexBuffer);
        if (binBuffer.empty())
        {
            return "HEX_CONVERSION_ERROR";
        }
        return cryptoManager.SetCertificate(certId, binBuffer);
    }
    else if ((cmd[0] == 'k') && (cmd.size() > 1))
    {
        std::string hexBuffer = cmd.substr(1);
        std::string binBuffer = HexToBytes(hexBuffer);
        if (binBuffer.empty())
        {
            return "HEX_CONVERSION_ERROR";
        }
        return cryptoManager.SetKeyRSA(binBuffer);
    }
    else if ((cmd[0] == '@') && (cmd.size() == 1))
    {
        if (cryptoManager.CleanCrypto())
        {
            return "OK";
        }
        return "OP_ERROR";
    }
    else if ((cmd[0] == 'i') && (cmd.size() == 1))
    {
        return cryptoManager.GetStoredCertsAndKeys();
    }
    return "SYNTAX_ERROR";
}

std::string CommandManager::CommandCryptoOpen(const std::string &cmd)
{
    std::string binBuffer = HexToBytes(cmd);
    if (binBuffer.size() < 2)
    {
        return "BUFFER_TOO_SHORT";
    }

    uint16_t sigLen = (static_cast<unsigned char>(binBuffer[0]) << 8) | static_cast<unsigned char>(binBuffer[1]);
    if (binBuffer.size() < 2 + sigLen)
    {
        return "INVALID_BUFFER";
    }

    std::vector<uint8_t> signature(binBuffer.begin() + 2, binBuffer.begin() + 2 + sigLen);
    std::vector<uint8_t> ciphertext(binBuffer.begin() + 2 + sigLen, binBuffer.end());
    std::string          apiPubKey = GetAPIPublicKey();
    if (apiPubKey.empty())
    {
        return "API_CERT_ERROR";
    }

    bool verified = cryptoManager.VerifySignature(apiPubKey, ciphertext, signature);
    if (!verified)
    {
        return "SIGNATURE_ERROR";
    }

    std::vector<uint8_t> decrypted;
    bool                 decryptedOk = cryptoManager.DecryptWithPrivateKey(ciphertext, decrypted);
    if (!decryptedOk)
    {
        return "DECRYPTION_ERROR";
    }

    std::string message(decrypted.begin(), decrypted.end());
    return message;
}

std::string CommandManager::CommandRsaKeySize(const std::string &cmd)
{
    int newSize = std::stoi(cmd);
    if (newSize != 1024 && newSize != 2048)
    {
        return "INVALID_RSA_KEY_SIZE";
    }
    if (!cryptoManager.CanChangeKeySize())
    {
        return "OPERATION_NOT_ALLOWED";
    }
    bool res = cryptoManager.SetRsaKeySize(static_cast<uint16_t>(newSize));
    return res ? "OK" : "OPERATION_FAILED";
}

std::string CommandManager::HexToBytes(const std::string &hex)
{
    std::string cleaned;
    for (char c : hex)
    {
        if (!std::isspace(static_cast<unsigned char>(c)))
        {
            cleaned.push_back(c);
        }
    }
    if (cleaned.size() % 2 != 0)
        return {};

    std::string bytes;
    bytes.reserve(cleaned.size() / 2);
    for (size_t i = 0; i < cleaned.size(); i += 2)
    {
        std::string byteString = cleaned.substr(i, 2);
        char        byteValue  = static_cast<char>(strtol(byteString.c_str(), nullptr, 16));
        bytes.push_back(byteValue);
    }
    return bytes;
}
