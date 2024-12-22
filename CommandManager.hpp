#ifndef COMMAND_MANAGER_HPP
#define COMMAND_MANAGER_HPP

#include <Arduino.h>

/**
 * @class CommandManager
 * @brief Handles processing and execution of commands received via serial input.
 */
class CommandManager
{
  public:
    /**
     * @brief Constructor for CommandManager.
     */
    CommandManager();

    /**
     * @brief Initializes the CommandManager and associated components.
     */
    void Init();

    /**
     * @brief Processes a given command string.
     *
     * @param cmd The command string to process.
     */
    void ProcessCommand(String cmd);

  private:
    /**
     * @brief Processes test-related commands.
     *
     * @param cmd The test command string to process.
     * @param code Reference to a string where the result code will be stored.
     */
    void CommandTests(String cmd, String &code);
};

#endif
