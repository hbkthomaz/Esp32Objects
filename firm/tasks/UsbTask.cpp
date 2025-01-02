#include "UsbTask.hpp"
#include "driver/uart.h"
#include "CommandManager.hpp"
#include <string>

#define BUF_SIZE (1024)
#define UART_NUM UART_NUM_0

CommandManager commandManager;

void UsbTask(void *param)
{
    if (param == NULL)
    {
        printf("Error: UART queue handle is NULL\n");
        vTaskDelete(NULL);
    }

    QueueHandle_t queue = *(QueueHandle_t *)param;
    if (queue == NULL)
    {
        printf("Error: Invalid UART queue handle\n");
        vTaskDelete(NULL);
    }

    uart_event_t event;
    std::string  inputString;
    uint8_t      data[BUF_SIZE];
    commandManager.Init();

    while (true)
    {
        if (xQueueReceive(queue, &event, portMAX_DELAY))
        {
            if (event.type == UART_DATA)
            {
                int len = uart_read_bytes(UART_NUM, data, (event.size > BUF_SIZE ? BUF_SIZE : event.size), portMAX_DELAY);
                if (len > 0)
                {
                    for (int i = 0; i < len; i++)
                    {
                        char inChar = (char)data[i];
                        if (inChar == '\n')
                        {
                            std::string code = commandManager.ProcessCommand(inputString);
                            printf("%s\n", code.c_str());
                            inputString.clear();
                        }
                        else
                        {
                            inputString.push_back(inChar);
                        }
                    }
                }
            }
        }
    }
}

void UsbTaskCreate()
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    esp_err_t err = uart_param_config(UART_NUM, &uart_config);
    if (err != ESP_OK)
    {
        printf("Failed to configure UART parameters: %s\n", esp_err_to_name(err));
        return;
    }

    QueueHandle_t uartQueue;
    err = uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uartQueue, 0);
    if (err != ESP_OK)
    {
        return;
    }

    if (xTaskCreate(UsbTask, "UsbTask", 4096, &uartQueue, 10, NULL) != pdPASS)
    {
        uart_driver_delete(UART_NUM);
    }
}
