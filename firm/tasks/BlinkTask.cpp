#include "BlinkTask.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <string.h>

#define LED_PIN GPIO_NUM_2

void BlinkTask(void *param)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while (true)
    {
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void BlinkTaskCreate()
{
    xTaskCreate(BlinkTask, "Blink Task", 2048, NULL, 1, NULL);
}