#ifndef FLASH_MANAGER_HPP
#define FLASH_MANAGER_HPP

#include <string>

/**
 * @brief Class responsible for managing SPIFFS filesystem operations on the ESP32.
 */
class FlashManager
{
  public:
    /**
     * @brief Default constructor.
     */
    FlashManager();

    /**
     * @brief Initializes the SPIFFS filesystem.
     * @return `true` if initialization is successful, `false` otherwise.
     */
    bool Init();

    /**
     * @brief Processes a command and executes the corresponding action.
     * @param cmdIn The input command.
     * @return Response string after processing the command.
     *
     * Commands:
     * - `@`        : Deletes all stored data.
     * - `@<key>`   : Deletes data associated with the specified key (hexadecimal).
     * - `#`        : Returns all stored data.
     * - `<key>|<data>` : Stores or updates data for the given key.
     * - `<key>`    : Retrieves data associated with the specified key.
     */
    std::string HandleCommand(const std::string &cmdIn);

  private:
    /**
     * @brief Checks if a string consists only of hexadecimal characters.
     * @param s The string to be checked.
     * @return `true` if the string is hexadecimal, `false` otherwise.
     */
    bool isHex(const std::string &s) const;

    /**
     * @brief Writes or updates data in the SPIFFS file.
     * @param id Numeric identifier.
     * @param data Data string to be written.
     * @return `true` if the operation is successful, `false` otherwise.
     */
    bool writeData(long id, const std::string &data);

    /**
     * @brief Reads the entire content of the SPIFFS file.
     * @return Complete file content as a string. Returns an empty string on error.
     */
    std::string readAllData() const;

    /**
     * @brief Reads data corresponding to a specific ID.
     * @param id Numeric identifier.
     * @return Found data as a string or an empty string if not found.
     */
    std::string readDataById(long id) const;

    /**
     * @brief Deletes data associated with a specific ID.
     * @param id Numeric identifier.
     * @return `true` if the operation is successful, `false` otherwise.
     */
    bool deleteDataById(long id);

    /**
     * @brief Deletes the SPIFFS file.
     * @return `true` if the operation is successful, `false` otherwise.
     */
    bool deleteFile();

    /**
     * @brief Creates a new SPIFFS file if it does not exist.
     * @return `true` if the operation is successful, `false` otherwise.
     */
    bool createFile();

    static constexpr const char *MOUNT_POINT = "/spiffs";
};

#endif // FLASH_MANAGER_HPP
