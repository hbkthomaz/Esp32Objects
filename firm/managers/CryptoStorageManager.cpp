#include "KeyStorageManager.hpp"
#include <cstdio>
#include <cstring>
#include <errno.h>

KeyStorageManager::KeyStorageManager()
{
}

bool KeyStorageManager::StoreCertificate(const std::string &fileName, const std::string &certData)
{
    FILE *file = fopen(fileName.c_str(), "wb");
    if (!file)
    {
        return false;
    }
    size_t written = fwrite(certData.data(), 1, certData.size(), file);
    fclose(file);
    return (written == certData.size());
}

bool KeyStorageManager::StoreKey(const std::string &fileName, const std::string &keyData)
{
    FILE *file = fopen(fileName.c_str(), "wb");
    if (!file)
    {
        return false;
    }
    size_t written = fwrite(keyData.data(), 1, keyData.size(), file);
    fclose(file);
    return (written == keyData.size());
}

std::string KeyStorageManager::LoadCertificate(const std::string &fileName) const
{
    FILE *file = fopen(fileName.c_str(), "rb");
    if (!file)
    {
        return "";
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    std::string data;
    data.resize(fileSize);
    size_t readBytes = fread(&data[0], 1, fileSize, file);
    fclose(file);
    if (readBytes != static_cast<size_t>(fileSize))
    {
        return "";
    }
    return data;
}

std::string KeyStorageManager::LoadKey(const std::string &fileName) const
{
    FILE *file = fopen(fileName.c_str(), "rb");
    if (!file)
    {
        return "";
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    std::string data;
    data.resize(fileSize);
    size_t readBytes = fread(&data[0], 1, fileSize, file);
    fclose(file);
    if (readBytes != static_cast<size_t>(fileSize))
    {
        return "";
    }
    return data;
}

bool KeyStorageManager::RemoveFile(const std::string &fileName)
{
    return (remove(fileName.c_str()) == 0 || errno == ENOENT);
}

bool KeyStorageManager::Exists(const std::string &fileName)
{
    FILE *file = fopen(fileName.c_str(), "rb");
    if (file)
    {
        fclose(file);
        return true;
    }
    return false;
}
