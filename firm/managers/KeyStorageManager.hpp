#ifndef KEY_STORAGE_MANAGER_HPP
#define KEY_STORAGE_MANAGER_HPP

#include <string>

/**
 * @brief Manages storage operations for keys and certificates.
 *
 * This class provides methods for storing, loading, removing, and checking the existence
 * of keys and certificates in the file system.
 */
class KeyStorageManager
{
  public:
    /**
     * @brief Constructs a new KeyStorageManager object.
     */
    KeyStorageManager();

    /**
     * @brief Stores a certificate to the specified file.
     *
     * @param fileName The path to the file where the certificate will be stored.
     * @param certData The certificate data as a string.
     * @return true if the certificate was successfully stored, false otherwise.
     */
    bool StoreCertificate(const std::string &fileName, const std::string &certData);

    /**
     * @brief Stores a key to the specified file.
     *
     * @param fileName The path to the file where the key will be stored.
     * @param keyData The key data as a string.
     * @return true if the key was successfully stored, false otherwise.
     */
    bool StoreKey(const std::string &fileName, const std::string &keyData);

    /**
     * @brief Loads a certificate from the specified file.
     *
     * @param fileName The path to the file containing the certificate.
     * @return The certificate data as a string, or an empty string if an error occurred.
     */
    std::string LoadCertificate(const std::string &fileName) const;

    /**
     * @brief Loads a key from the specified file.
     *
     * @param fileName The path to the file containing the key.
     * @return The key data as a string, or an empty string if an error occurred.
     */
    std::string LoadKey(const std::string &fileName) const;

    /**
     * @brief Removes the specified file.
     *
     * @param fileName The path to the file to be removed.
     * @return true if the file was successfully removed or does not exist, false otherwise.
     */
    bool RemoveFile(const std::string &fileName);

    /**
     * @brief Checks if a file exists.
     *
     * @param fileName The path to the file to check.
     * @return true if the file exists, false otherwise.
     */
    bool Exists(const std::string &fileName);
};

#endif // KEY_STORAGE_MANAGER_HPP
