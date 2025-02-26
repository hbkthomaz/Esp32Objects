#include "CommandManager.hpp"
#include "FlashManager.hpp"
#include "CryptoManager.hpp"
#include <algorithm>

static FlashManager flashManager;
static CryptoManager cryptoManager;

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
    std::string code;

    if (cmd.empty())
    {
        code = "SYNTAX_ERROR";
        return code;
    }

    if ((cmd[0] == 't')&&(cmd.size()>1))
    {
        return CommandTests(cmd.substr(1));
    }
    else if((cmd[0] == 'c')&&(cmd.size()>1))
    {
        return CommandCryptoSet(cmd.substr(1));

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
std::string CommandManager::CommandCryptoSet(const std::string &cmd)
{
    if (cmd.size() < 2)
    {
        return "SYNTAX_ERROR";
    }
    if ((cmd[0] == 'c')&&(cmd.size()>2))
    {
        char             certIdChar = cmd[1];
        int              certIdInt  = static_cast<int>(certIdChar - '0');
        certificateId_et certId     = static_cast<certificateId_et>(certIdInt);
        std::string      buffer     = cmd.substr(2);
        return cryptoManager.CertificateSet(certId, buffer, static_cast<uint32_t>(buffer.size()));
    }
    else if ((cmd[0] == 'k')&&(cmd.size()>1))
    {
        std::string buffer = cmd.substr(1);
        return cryptoManager.KeySetRSA(buffer, static_cast<uint32_t>(buffer.size()));
    }
    return "SYNTAX_ERROR";
}
