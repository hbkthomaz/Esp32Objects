#include "CommandManager.hpp"
#include "FlashManager.hpp"
#include <algorithm>

static FlashManager flashManager;

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
    std::string code;

    if (cmd.empty())
    {
        code = "SYNTAX_ERROR";
        return code;
    }

    if (cmd[0] == 't')
    {
        return CommandTests(cmd);
    }

    return "SYNTAX_ERROR";
}

std::string CommandManager::CommandTests(const std::string &cmd)
{
    if (cmd.size() < 2)
    {
        return "SYNTAX_ERROR";
    }
    char        subCmd       = cmd[1];
    std::string remainingCmd = cmd.substr(2);

    if (subCmd == 'f')
    {
        return flashManager.HandleCommand(remainingCmd);
    }

    return "SYNTAX_ERROR";
}
