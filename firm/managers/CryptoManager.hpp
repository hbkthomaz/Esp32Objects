#ifndef CRYPTO_MANAGER_HPP
#define CRYPTO_MANAGER_HPP

#include <string>

typedef enum
{
    CA,
    DEVICE,
    API
} certificateId_et;

class CryptoManager
{
  public:
    CryptoManager();

    bool Init();
    std::string CertificateSet(certificateId_et id,const std::string &buffer,uint32_t size);
    std::string KeySetRSA(const std::string &buffer,uint32_t size);

  private:
    bool CreateFile();
};

#endif // CRYPTO_MANAGER_HPP
