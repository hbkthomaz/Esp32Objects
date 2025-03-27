#include "CommandManager.hpp"
#include "FlashManager.hpp"
#include "CryptoManager.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>

static FlashManager  flashManager;
static CryptoManager cryptoManager;

CommandManager::CommandManager()
{
}

void CommandManager::Init()
{
    flashManager.Init();
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
        char             certIdChar = cmd[1];
        int              certIdInt  = static_cast<int>(certIdChar - '0');
        certificateId_et certId     = static_cast<certificateId_et>(certIdInt);
        return cryptoManager.CertificateGet(certId);
    }
    else if ((cmd[0] == 'k') && (cmd.size() >= 2))
    {
        return cryptoManager.KeyGetRSA();
    }
    return "SYNTAX_ERROR";
}
std::string CommandManager::CommandCryptoSet(const std::string &cmd)
{
    if ((cmd[0] == 'c') && (cmd.size() > 2))
    {
        char             certIdChar = cmd[1];
        int              certIdInt  = static_cast<int>(certIdChar - '0');
        certificateId_et certId     = static_cast<certificateId_et>(certIdInt);
        std::string      hexBuffer  = cmd.substr(2);
        std::string      binBuffer  = HexToBytes(hexBuffer);
        if (binBuffer.empty())
        {
            return "HEX_CONVERSION_ERROR";
        }
        return cryptoManager.CertificateSet(certId, binBuffer, static_cast<uint32_t>(binBuffer.size()));
    }
    else if ((cmd[0] == 'k') && (cmd.size() > 1))
    {
        std::string hexBuffer = cmd.substr(1);
        std::string binBuffer = HexToBytes(hexBuffer);
        if (binBuffer.empty())
        {
            return "HEX_CONVERSION_ERROR";
        }
        return cryptoManager.KeySetRSA(binBuffer, static_cast<uint32_t>(binBuffer.size()));
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
