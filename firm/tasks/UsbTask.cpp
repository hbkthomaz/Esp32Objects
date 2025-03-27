#include "UsbTask.hpp"
#include "driver/uart.h"
#include "CommandManager.hpp"
#include <string>
#include <cstring>
#include <stdlib.h>

#define BUF_SIZE (8192)
#define UART_NUM UART_NUM_0

CommandManager commandManagerUsb;

static void UsbTask(void *param);

void UsbTaskCreate()
{
    uart_config_t uart_config = {.baud_rate           = 115200,
                                 .data_bits           = UART_DATA_8_BITS,
                                 .parity              = UART_PARITY_DISABLE,
                                 .stop_bits           = UART_STOP_BITS_1,
                                 .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
                                 .rx_flow_ctrl_thresh = 122,
                                 .source_clk          = UART_SCLK_DEFAULT,
                                 .flags               = 0};

    esp_err_t err = uart_param_config(UART_NUM, &uart_config);
    if (err != ESP_OK)
    {
        return;
    }

    QueueHandle_t uartQueue;
    err = uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uartQueue, 0);
    if (err != ESP_OK)
    {
        return;
    }

    if (xTaskCreate(UsbTask, "UsbTask", 16384, &uartQueue, 5, NULL) != pdPASS)
    {
        uart_driver_delete(UART_NUM);
    }
}

static void UsbTask(void *param)
{
    if (param == NULL)
    {
        vTaskDelete(NULL);
    }

    QueueHandle_t queue = *(QueueHandle_t *)param;
    if (queue == NULL)
    {
        vTaskDelete(NULL);
    }

    uart_event_t event;
    char        *inputBuffer = (char *)malloc(BUF_SIZE);
    if (inputBuffer == NULL)
    {
        vTaskDelete(NULL);
    }
    memset(inputBuffer, 0, BUF_SIZE);
    size_t inputIndex = 0;

    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    if (data == NULL)
    {
        free(inputBuffer);
        vTaskDelete(NULL);
    }

    commandManagerUsb.Init();

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
                        char inChar = static_cast<char>(data[i]);
                        if (inChar == '\n')
                        {
                            inputBuffer[inputIndex] = '\0';
                            std::string command(inputBuffer);
                            std::string response = commandManagerUsb.ProcessCommand(command);
                            printf("%s\n", response.c_str());
                            inputIndex = 0;
                        }
                        else
                        {
                            if (inputIndex < BUF_SIZE - 1)
                            {
                                inputBuffer[inputIndex++] = inChar;
                            }
                            else
                            {
                                inputIndex = 0;
                            }
                        }
                    }
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
            }
        }
    }
    free(data);
    free(inputBuffer);
    vTaskDelete(NULL);
}
