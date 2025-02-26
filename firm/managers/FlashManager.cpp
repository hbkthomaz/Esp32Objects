#include "FlashManager.hpp"

#include <cstdio>
#include <cstring>

extern "C"
{
#include "esp_err.h"
#include "esp_spiffs.h"
#include "esp_log.h"
}

#define DATA_FILE "flash.txt"
static constexpr const char *MOUNT_POINT = "/spiffs";
FlashManager::FlashManager()
{
}

bool FlashManager::Init()
{
    esp_vfs_spiffs_conf_t conf = {.base_path = MOUNT_POINT, .partition_label = nullptr, .max_files = 5, .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(nullptr, &total, &used);

    if (!CreateFile())
    {
        return false;
    }

    return true;
}

bool FlashManager::IsHex(const std::string &s) const
{
    if (s.empty())
        return false;
    for (char c : s)
    {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
        {
            return false;
        }
    }
    return true;
}

std::string FlashManager::HandleCommand(const std::string &cmdIn)
{
    std::string cmd = cmdIn;
    cmd.erase(cmd.find_last_not_of("\r\n") + 1);
    std::string code = "SYNTAX_ERROR";
    do
    {
        if (cmd.empty())
        {
            break;
        }
        if (cmd[0] == '@')
        {
            if (cmd.size() <= 1)
            {
                if (DeleteFile() && CreateFile())
                {
                    code = "OK";
                    break;
                }
                code = "OP_ERROR";
                break;
            }
            std::string keyHex = cmd.substr(1);
            if (!IsHex(keyHex))
            {
                break;
            }
            long id = strtol(keyHex.c_str(), nullptr, 16);
            if (DeleteDataById(id))
            {
                code = "OK";
                break;
            }
            code = "OP_ERROR";
            break;
        }
        if (cmd == "#")
        {
            std::string all = ReadAllData();
            if (all.empty())
            {
                code = "OP_ERROR";
                break;
            }
            code = all;
            break;
        }
        size_t pos = cmd.find('|');
        if (pos != std::string::npos)
        {
            std::string hexPart  = cmd.substr(0, pos);
            std::string dataPart = cmd.substr(pos + 1);
            if (!IsHex(hexPart))
            {
                break;
            }
            if (dataPart.empty())
            {
                break;
            }
            long id = strtol(hexPart.c_str(), nullptr, 16);
            if (WriteData(id, dataPart))
            {
                code = "OK";
                break;
            }
            code = "OP_ERROR";
            break;
        }
        if (!IsHex(cmd))
        {
            break;
        }
        long        id    = strtol(cmd.c_str(), nullptr, 16);
        std::string found = ReadDataById(id);
        if (found.empty())
        {
            code = "OP_ERROR";
            break;
        }
        code = found;
    } while (0);
    return code;
}

bool FlashManager::WriteData(long id, const std::string &data)
{
    FILE *f = fopen((std::string(MOUNT_POINT) + "/" + DATA_FILE).c_str(), "r");
    if (!f)
    {
        return false;
    }

    std::string content;
    bool        found = false;
    char        lineBuf[256];

    while (fgets(lineBuf, sizeof(lineBuf), f))
    {
        std::string line(lineBuf);
        if (!line.empty() && line.back() == '\n')
        {
            line.pop_back();
        }

        std::string match = "c$" + std::to_string(id) + "$";
        if (line.rfind(match, 0) == 0)
        {
            content += match + data + "\n";
            found = true;
        }
        else
        {
            content += line + "\n";
        }
    }
    fclose(f);

    if (!found)
    {
        content += "c$" + std::to_string(id) + "$" + data + "\n";
    }

    f = fopen((std::string(MOUNT_POINT) + "/" + DATA_FILE).c_str(), "w");
    if (!f)
    {
        return false;
    }

    if (fputs(content.c_str(), f) == EOF)
    {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

std::string FlashManager::ReadAllData() const
{
    FILE *f = fopen((std::string(MOUNT_POINT) + "/" + DATA_FILE).c_str(), "r");
    if (!f)
    {
        return "";
    }

    std::string result;
    char        lineBuf[256];

    while (fgets(lineBuf, sizeof(lineBuf), f))
    {
        std::string line(lineBuf);
        if (!line.empty() && line.back() == '\n')
        {
            line.pop_back();
        }
        if (!line.empty())
        {
            result += line + "\n";
        }
    }
    fclose(f);

    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }

    return result;
}

std::string FlashManager::ReadDataById(long id) const
{
    FILE *f = fopen((std::string(MOUNT_POINT) + "/" + DATA_FILE).c_str(), "r");
    if (!f)
    {
        return "";
    }

    std::string match = "c$" + std::to_string(id) + "$";
    char        lineBuf[256];
    std::string found;

    while (fgets(lineBuf, sizeof(lineBuf), f))
    {
        std::string line(lineBuf);
        if (!line.empty() && line.back() == '\n')
        {
            line.pop_back();
        }

        if (line.rfind(match, 0) == 0)
        {
            found = line.substr(match.size());
            break;
        }
    }
    fclose(f);

    if (!found.empty())
    {
    }
    else
    {
    }

    return found;
}

bool FlashManager::DeleteFile()
{
    std::string filePath = std::string(MOUNT_POINT) + "/" + DATA_FILE;
    if (remove(filePath.c_str()) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool FlashManager::CreateFile()
{
    std::string filePath = std::string(MOUNT_POINT) + "/" + DATA_FILE;
    FILE       *f        = fopen(filePath.c_str(), "r");
    if (!f)
    {
        f = fopen(filePath.c_str(), "w");
        if (!f)
        {
            return false;
        }
        fclose(f);
    }
    else
    {
        fclose(f);
    }
    return true;
}
bool FlashManager::DeleteDataById(long id)
{
    FILE *f = fopen((std::string(MOUNT_POINT) + "/" + DATA_FILE).c_str(), "r");
    if (!f)
    {
        return false;
    }

    std::string content;
    std::string match = "c$" + std::to_string(id) + "$";
    char        lineBuf[256];
    bool        found = false;

    while (fgets(lineBuf, sizeof(lineBuf), f))
    {
        std::string line(lineBuf);
        if (!line.empty() && line.back() == '\n')
        {
            line.pop_back();
        }

        if (line.rfind(match, 0) == 0)
        {
            found = true;
            continue;
        }
        content += line + "\n";
    }
    fclose(f);

    if (!found)
    {
        return false;
    }

    f = fopen((std::string(MOUNT_POINT) + "/" + DATA_FILE).c_str(), "w");
    if (!f)
    {
        return false;
    }

    if (fputs(content.c_str(), f) == EOF)
    {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}
