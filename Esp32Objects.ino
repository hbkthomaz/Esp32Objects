#include <SPIFFS.h>
#include <Arduino.h>
#include "CommandManager.hpp"

CommandManager commandManager;

void setup()
{
    Serial.begin(115200);
    commandManager.Init();
}

void loop()
{
    static String inputString = "";
    static bool   receiving   = false;

    while (Serial.available())
    {
        char inChar = (char)Serial.read();
        if (inChar == '\n')
        {
            commandManager.ProcessCommand(inputString);
            inputString = "";
            receiving   = false;
        }
        else
        {
            inputString += inChar;
            receiving = true;
        }
        yield();
    }
}
