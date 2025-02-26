#include "CryptoManager.hpp"

#include <cstdio>
#include <cstring>

extern "C"
{
#include "esp_err.h"
#include "esp_spiffs.h"
#include "esp_log.h"
}

#define DATA_FILE "crypto.txt"
static constexpr const char *MOUNT_POINT = "/spiffs";

CryptoManager::CryptoManager()
{
}

bool CryptoManager::Init()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path              = MOUNT_POINT,
        .partition_label        = nullptr,
        .max_files              = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        return false;
    }

    size_t total = 0;
    size_t used  = 0;
    ret = esp_spiffs_info(nullptr, &total, &used);

    // Optionally check the ret value from esp_spiffs_info
    // e.g., if (ret != ESP_OK) { ... }

    if (!CreateFile())
    {
        return false;
    }

    return true;
}

bool CryptoManager::CreateFile()
{
    std::string filePath = std::string(MOUNT_POINT) + "/" + DATA_FILE;
    FILE *f = fopen(filePath.c_str(), "r");
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

std::string CryptoManager::CertificateSet(certificateId_et id, const std::string &buffer, uint32_t size)
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
        return "INVALID_CERTIFICATE_ID";
    }

    std::string filePath = std::string(MOUNT_POINT) + "/" + fileName;
    std::string data= "certificate$" + std::to_string(id) + "$"+buffer.c_str()+ "\n";
    FILE *f = fopen(filePath.c_str(), "w");
    if (!f)
    {
        return "OP_ERROR";
    }

    if(fputs(data.c_str(), f) == EOF)
    {
        fclose(f);
        return "OP_ERROR";
    }
    fclose(f);
    return "OK";
}

std::string CryptoManager::KeySetRSA(const std::string &buffer, uint32_t size)
{
    std::string fileName = "device.key";
    std::string filePath = std::string(MOUNT_POINT) + "/" + fileName;

    FILE *f = fopen(filePath.c_str(), "w");
    if (!f)
    {
        return "OP_ERROR";
    }

    size_t written = fwrite(buffer.c_str(), 1, size, f);
    fclose(f);

    if (written != size)
    {
        return "OP_ERROR";
    }

    return filePath;
}
