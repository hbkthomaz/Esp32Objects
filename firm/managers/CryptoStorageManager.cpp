#include "CryptoStorageManager.hpp"
#include <cstdio>
#include <cstring>
#include <errno.h>

CryptoStorageManager::CryptoStorageManager()
{
}

bool CryptoStorageManager::StoreData(const std::string &fileName, const std::string &data)
{
    FILE *file = fopen(fileName.c_str(), "wb");
    if (!file)
    {
        return false;
    }
    size_t written = fwrite(data.data(), 1, data.size(), file);
    fclose(file);
    return (written == data.size());
}

std::string CryptoStorageManager::LoadData(const std::string &fileName) const
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

bool CryptoStorageManager::RemoveFile(const std::string &fileName)
{
    return (remove(fileName.c_str()) == 0 || errno == ENOENT);
}

bool CryptoStorageManager::Exists(const std::string &fileName)
{
    FILE *file = fopen(fileName.c_str(), "rb");
    if (file)
    {
        fclose(file);
        return true;
    }
    return false;
}
