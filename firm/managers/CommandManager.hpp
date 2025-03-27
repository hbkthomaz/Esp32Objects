#ifndef COMMAND_MANAGER_HPP
#define COMMAND_MANAGER_HPP

#include <string>

/**
 * @class CommandManager
 * @brief Routes command strings and triggers the appropriate handlers.
 */
class CommandManager
{
  public:
    /**
     * @brief Constructs a CommandManager.
     */
    CommandManager();

    /**
     * @brief Performs setup procedures (e.g., flash initialization).
     */
    void Init();

    /**
     * @brief Parses and executes a given command string.
     * @param cmdOriginal The unprocessed command.
     * @return A string result code or "SYNTAX_ERROR".
     */
    std::string ProcessCommand(const std::string &cmdOriginal);

  private:
    /**
     * @brief Handles test-related commands.
     * @param cmd Subcommand string.
     * @return A string result code or "SYNTAX_ERROR".
     */
    std::string CommandTests(const std::string &cmd);

    /**
     * @brief Handles certificate/key storage and related operations.
     * @param cmd Subcommand string.
     * @return A string result code or "SYNTAX_ERROR".
     */
    std::string CommandCryptoSet(const std::string &cmd);

    /**
     * @brief Retrieves stored certificates or keys in hex format.
     * @param cmd Subcommand string.
     * @return Hex data or "SYNTAX_ERROR".
     */
    std::string CommandCryptoGet(const std::string &cmd);

    /**
     * @brief Converts a hex string to its byte representation.
     * @param hex Input hex string.
     * @return Corresponding byte sequence, or empty on failure.
     */
    std::string HexToBytes(const std::string &hex);
};

#endif // COMMAND_MANAGER_HPP
