# ESP32 Project

This project uses **ESP-IDF v5.3** for development.

## Requirements

1. Install ESP-IDF v5.3: Follow the official ESP-IDF installation guide at:
   [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
2. Set up the environment by running:
   ```bash
   . $IDF_PATH/export.sh
   ```
   Alternatively, you can use the ESP-IDF PowerShell or bash environment.

## How to Build and Flash
Inside the firm folder
### Build the Project
To build the firmware, run:
```bash
idf.py build
```

### Flash the Firmware
Connect your ESP32 to your computer and find the correct serial port (e.g., `COM6` on Windows). Then, flash the firmware using:
```bash
idf.py -p COM6 flash
```

Replace `COM6` with the appropriate port for your device.

### Monitor the Output
To monitor the ESP32 output, use:
```bash
idf.py -p COM6 monitor
```

Exit the monitor using `Ctrl+]`.

## Useful Commands

- **Clean the build directory**:
  ```bash
  idf.py clean
  ```
- **Rebuild the project from scratch**:
  ```bash
  idf.py fullclean
  ```

## Notes

- Ensure ESP-IDF v5.3 is properly installed and set up.
- For more information, check the ESP-IDF documentation at:
  [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html)
