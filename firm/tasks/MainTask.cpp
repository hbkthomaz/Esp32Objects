#include "MainTask.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "BlinkTask.hpp"
#include "UsbTask.hpp"

void MainTask(void *param)
{
    BlinkTaskCreate();
    UsbTaskCreate();
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void MainTaskCreate()
{
    xTaskCreate(MainTask, "Main Task", 2048, NULL, 5, NULL);
}