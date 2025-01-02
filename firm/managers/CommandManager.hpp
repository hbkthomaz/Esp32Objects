#ifndef COMMAND_MANAGER_HPP
#define COMMAND_MANAGER_HPP

#include <string>

/**
 * @class CommandManager
 * @brief Handles command processing and execution.
 */
class CommandManager
{
  public:
    /**
     * @brief Default constructor.
     */
    CommandManager();

    /**
     * @brief Initializes the CommandManager.
     */
    void Init();

    /**
     * @brief Processes a command string.
     * @param cmdOriginal Original command string.
     * @return Result code after processing the command.
     */
    std::string ProcessCommand(const std::string &cmdOriginal);

  private:
    /**
     * @brief Processes test-related commands.
     * @param cmd Command string.
     * @return Result code after processing the test command.
     */
    std::string CommandTests(const std::string &cmd);
};

#endif // COMMAND_MANAGER_HPP
