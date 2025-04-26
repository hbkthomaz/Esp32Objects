#ifndef COMMAND_MANAGER_HPP
#define COMMAND_MANAGER_HPP

#include <string>

/**
 * @class CommandManager
 * @brief Routes command strings and triggers the appropriate handlers.
 *
 * The CommandManager class parses incoming command strings and delegates them to the
 * appropriate subsystem handlers, such as flash operations and cryptographic commands.
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
     *
     * This function trims and processes the incoming command string, delegating the work
     * to specific command handlers based on the command's prefix.
     *
     * @param cmdOriginal The unprocessed command string.
     * @return A string result code, decrypted message, or "SYNTAX_ERROR" on failure.
     */
    std::string ProcessCommand(const std::string &cmdOriginal);

  private:
    /**
     * @brief Handles test-related commands.
     *
     * Delegates test commands (e.g., flash tests) to the appropriate handler.
     *
     * @param cmd Subcommand string.
     * @return A string result code or "SYNTAX_ERROR" if the subcommand is unrecognized.
     */
    std::string CommandTests(const std::string &cmd);

    /**
     * @brief Handles certificate/key storage and related operations.
     *
     * Processes commands for storing certificates and keys in the system.
     *
     * @param cmd Subcommand string.
     * @return A string result code indicating success or error (e.g., "OK", "HEX_CONVERSION_ERROR").
     */
    std::string CommandCryptoSet(const std::string &cmd);

    /**
     * @brief Retrieves stored certificates or keys in hex format.
     *
     * This function handles commands that request the retrieval of stored cryptographic data.
     *
     * @param cmd Subcommand string.
     * @return The hex-encoded certificate/key data, or "SYNTAX_ERROR" if the request is invalid.
     */
    std::string CommandCryptoGet(const std::string &cmd);

    /**
     * @brief Processes a cryptographic "open" command.
     *
     * This command expects a hex-encoded buffer that contains the following structure:
     * [2 bytes signature length (big-endian)][signature][ciphertext].
     * The function performs the following steps:
     * - Converts the hex input to a binary buffer.
     * - Extracts the signature and ciphertext.
     * - Loads the API public key from the API certificate.
     * - Verifies the signature over the ciphertext using the API public key.
     * - Decrypts the ciphertext using the device's private key.
     * - Returns the decrypted plaintext message.
     *
     * @param cmd Subcommand string (hex-encoded buffer, excluding the command identifier).
     * @return The decrypted plaintext message on success, or an error code on failure.
     */
    std::string CommandCryptoOpen(const std::string &cmd);

    /**
     * @brief Processes a command to set the RSA key size for future key generation.
     *
     * The command expects a numeric parameter (e.g., "1024" or "2048"). Changing the RSA key
     * size is permitted only if neither the CA certificate nor the device key is already stored.
     *
     * @param cmd Subcommand string containing the desired RSA key size.
     * @return "OK" if the key size was updated successfully, or an error code string otherwise.
     */
    std::string CommandRsaKeySize(const std::string &cmd);

    /**
     * @brief Converts a hex string to its byte representation.
     *
     * Removes any whitespace and converts the resulting hex string into a binary buffer.
     *
     * @param hex Input hex string.
     * @return Corresponding byte sequence, or an empty string on failure.
     */
    std::string HexToBytes(const std::string &hex);
};

#endif // COMMAND_MANAGER_HPP
