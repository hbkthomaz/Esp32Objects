#include "FlashManager.hpp"
#include <SPIFFS.h>

FlashManagerClass::FlashManagerClass()
{
}
bool FlashManagerClass::Init()
{
    bool ret = false;
    do
    {
        if (!SPIFFS.begin(true))
        {
            break;
        }

        if (!SPIFFS.exists("/data.txt"))
        {
            File file = SPIFFS.open("/data.txt", FILE_WRITE);
            if (!file)
            {
                break;
            }
            file.close();
        }
        ret = true;
    } while (0);
}
String FlashManagerClass::HandleCommand(String cmd)
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

        if (cmd.equals("@"))
        {
            if (DeleteFile())
            {
                if (CreateFile())
                {
                    code = "OK";
                }
                else
                {
                    code = "OP_ERROR";
                }
            }
            else
            {
                code = "OP_ERROR";
            }
            break;
        }

        if (cmd.equals("#"))
        {
            String result = ReadAllData();
            if (result.length() == 0)
            {
                code = "OP_ERROR";
            }
            else
            {
                code = result;
            }
            break;
        }

        int delimiterPos = cmd.indexOf('|');
        if (delimiterPos != -1)
        {
            String hexPart  = cmd.substring(0, delimiterPos);
            String dataPart = cmd.substring(delimiterPos + 1);

            bool isHex = true;
            for (unsigned int i = 0; i < hexPart.length(); i++)
            {
                char c = hexPart.charAt(i);
                if (!isxdigit(c))
                {
                    isHex = false;
                    break;
                }
            }

            if (!isHex || hexPart.length() == 0)
            {
                code = "SYNTAX_ERROR";
                break;
            }

            long id = strtol(hexPart.c_str(), NULL, 16);

            if (dataPart.length() > 0)
            {
                if (WriteData(id, dataPart))
                {
                    code = "OK";
                }
                else
                {
                    code = "OP_ERROR";
                }
            }
            else
            {
                code = "SYNTAX_ERROR";
            }
            break;
        }

        String hexPart = cmd;

        bool isHex = true;
        for (unsigned int i = 0; i < hexPart.length(); i++)
        {
            char c = hexPart.charAt(i);
            if (!isxdigit(c))
            {
                isHex = false;
                break;
            }
        }

        if (!isHex || hexPart.length() == 0)
        {
            code = "SYNTAX_ERROR";
            break;
        }

        long id = strtol(hexPart.c_str(), NULL, 16);

        String foundData = ReadDataById(id);

        if (foundData.length() > 0)
        {
            code = foundData;
        }
        else
        {
            code = "OP_ERROR";
        }

    } while (0);

    return code;
}

bool FlashManagerClass::WriteData(long id, String data)
{
    File   file    = SPIFFS.open("/data.txt", FILE_READ);
    String content = "";
    bool   found   = false;
    if (file)
    {
        while (file.available())
        {
            String line = file.readStringUntil('\n');
            if (line.startsWith("c$" + String(id) + "$"))
            {
                content += "c$" + String(id) + "$" + data + "\n";
                found = true;
            }
            else
            {
                content += line + "\n";
            }
            yield();
        }
        file.close();
    }
    if (!found)
    {
        content += "c$" + String(id) + "$" + data + "\n";
    }
    file = SPIFFS.open("/data.txt", FILE_WRITE);
    if (!file)
    {
        return false;
    }
    file.print(content);
    file.close();
    return true;
}

String FlashManagerClass::ReadAllData()
{
    File file = SPIFFS.open("/data.txt", FILE_READ);
    if (!file)
    {
        return "";
    }

    String result = "";
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        if (line.length() > 0)
        {
            result += line + "\n";
        }
        yield();
    }
    file.close();

    if (result.endsWith("\n"))
    {
        result.remove(result.length() - 1);
    }

    return result;
}

String FlashManagerClass::ReadDataById(long id)
{
    File file = SPIFFS.open("/data.txt", FILE_READ);
    if (!file)
    {
        return "";
    }

    String searchPrefix = "c$" + String(id) + "$";
    String foundData    = "";
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        if (line.startsWith(searchPrefix))
        {
            foundData = line.substring(searchPrefix.length());
            break;
        }
        yield();
    }
    file.close();

    return foundData;
}

bool FlashManagerClass::DeleteFile()
{
    return SPIFFS.remove("/data.txt");
}

bool FlashManagerClass::CreateFile()
{
    if (!SPIFFS.exists("/data.txt"))
    {
        File file = SPIFFS.open("/data.txt", FILE_WRITE);
        if (!file)
        {
            return false;
        }
        file.close();
        return true;
    }
    return true;
}
