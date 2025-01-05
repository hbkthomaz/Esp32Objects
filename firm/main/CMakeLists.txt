idf_component_register(SRCS 
    "main.cpp"
    "../tasks/BlinkTask.cpp"
    "../tasks/MainTask.cpp"
    "../tasks/UsbTask.cpp"
    "../managers/BleManager.cpp"
    "../managers/FlashManager.cpp"
    "../managers/CommandManager.cpp"
INCLUDE_DIRS "." "../tasks" "../managers")
