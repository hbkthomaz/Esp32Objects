#include "MainTask.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "BlinkTask.hpp"
#include "UsbTask.hpp"
#include "BleManager.hpp"

static void MainTask(void *param);


void MainTaskCreate()
{
    xTaskCreate(MainTask, "Main Task", 4096, NULL, 5, NULL);
}

static void MainTask(void *param)
{
    BlinkTaskCreate();
    UsbTaskCreate();
    BleManager bleManager;
    bleManager.Init();
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
