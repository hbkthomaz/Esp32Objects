#include "CommandManager.hpp"
#include "FlashManager.hpp"

CommandManager::CommandManager()
{
}
FlashManagerClass flashManager;
void              CommandManager::Init()
{
    flashManager.Init();
}

void CommandManager::ProcessCommand(String cmd)
{
    cmd.trim();
    String code = "";

    do
    {
        if (cmd.length() == 0)
        {
            code = "SYNTAX_ERROR";
            break;
        }

        if (cmd.startsWith("t"))
        {
            CommandTests(cmd, code);
            break;
        }

        code = "SYNTAX_ERROR";
    } while (0);

    Serial.println(code);
}

void CommandManager::CommandTests(String cmd, String &code)
{
    do
    {
        if (cmd.length() < 2)
        {
            code = "SYNTAX_ERROR";
            break;
        }

        char   subCmd       = cmd.charAt(1);
        String remainingCmd = cmd.substring(2);

        if (subCmd == 'f')
        {
            code = flashManager.HandleCommand(remainingCmd);
            break;
        }

        code = "SYNTAX_ERROR";
    } while (0);
}
