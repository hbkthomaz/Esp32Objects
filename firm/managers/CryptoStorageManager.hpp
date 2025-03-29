#ifndef CRYPTO_STORAGE_MANAGER_HPP
#define CRYPTO_STORAGE_MANAGER_HPP

#include <string>

/**
 * @brief Manages storage operations for keys and certificates.
 *
 * This class provides methods for storing, loading, removing, and checking the existence
 * of keys and certificates in the file system.
 */
class CryptoStorageManager
{
  public:
    /**
     * @brief Constructs a new CryptoStorageManager object.
     */
    CryptoStorageManager();

    /**
     * @brief Stores a key to the specified file.
     *
     * @param fileName The path to the file where the key will be stored.
     * @param keyData The data as a string.
     * @return true if the data was successfully stored, false otherwise.
     */
    bool StoreData(const std::string &fileName, const std::string &data);

    /**
     * @brief Loads a key from the specified file.
     *
     * @param fileName The path to the file containing the data.
     * @return The data as a string, or an empty string if an error occurred.
     */
    std::string LoadData(const std::string &fileName) const;

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

#endif // CRYPTO_STORAGE_MANAGER_HPP
