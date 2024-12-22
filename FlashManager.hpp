#ifndef FLASHMANAGER_HPP
#define FLASHMANAGER_HPP

#include <Arduino.h>
#include <FS.h>

/**
 * @class FlashManagerClass
 * @brief Manages flash storage operations using SPIFFS.
 */
class FlashManagerClass
{
  public:
    /**
     * @brief Constructor for FlashManagerClass.
     */
    FlashManagerClass();

    bool Init(void);
    /**
     * @brief Processes a received command and executes the corresponding action.
     *
     * @param cmd The command to process.
     * @return String The result of the operation.
     */
    String HandleCommand(String cmd);

    /**
     * @brief Updates an existing entry or appends a new formatted entry to the file.
     *
     * @param id The identifier in long format.
     * @param data The data to be stored.
     * @return true If the operation was successful.
     * @return false If an error occurred during the operation.
     */
    bool WriteData(long id, String data);

  private:
    /**
     * @brief Reads all data from the file.
     *
     * @return String The complete content of the file.
     */
    String ReadAllData(void);

    /**
     * @brief Reads data associated with a specific ID.
     *
     * @param id The identifier to search for.
     * @return String The found data or an empty string if not found.
     */
    String ReadDataById(long id);

    /**
     * @brief Deletes the data file.
     *
     * @return true If the file was successfully deleted.
     * @return false If an error occurred while deleting.
     */
    bool DeleteFile(void);

    /**
     * @brief Creates the data file if it does not exist.
     *
     * @return true If the file was created or already exists.
     * @return false If an error occurred while creating the file.
     */
    bool CreateFile(void);
};

#endif // FLASHMANAGER_HPP
