/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "freertos/portmacro.h"
#include "freertos/event_groups.h"
#include "input_iot.h"
#include "output_dev.h"





#define BIT_PRESS_SHORT (1<<0)
#define BIT_PRESS_NORMAL (1<<1)
#define BIT_PRESS_LONG (1<<2)

// static EventGroupHandle_t xCreatedEventGroup;
EventGroupHandle_t xCreatedEventGroup;

void input_event_callback (int pin, uint64_t tick)
{
    if (pin == BUTTON_GPIO)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        int press_ms  = tick * portTICK_PERIOD_MS;
        if (press_ms < 1000)
        {
            //press short
            xEventGroupSetBitsFromISR(xCreatedEventGroup, BIT_PRESS_SHORT, &xHigherPriorityTaskWoken);

        }
        else if (press_ms < 3000)
        {
            //press normal
            xEventGroupSetBitsFromISR(xCreatedEventGroup, BIT_PRESS_NORMAL, &xHigherPriorityTaskWoken);
        }
        else if (press_ms >= 3000)
        {
            //press long
            // xEventGroupSetBitsFromISR(xCreatedEventGroup, BIT_PRESS_LONG, &xHigherPriorityTaskWoken);
        }
    }

}

void button_timeout_callback(int pin)
{
    if (pin == GPIO_NUM_0)
    {
        printf("Timeout\n");
    }
}

void vTaskCode (void *pvParameters)
{
    for (;;)
    {
       
        EventBits_t uxBits = xEventGroupWaitBits(
                xCreatedEventGroup,   /* The event group being tested. */
                BIT_PRESS_SHORT | BIT_PRESS_NORMAL | BIT_PRESS_LONG, /* The bits within the event group to wait for. */
                pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
                pdFALSE,       /* Don't wait for both bits, either bit will do. */
                portMAX_DELAY );/* Wait a maximum of 100ms for either bit to be set. */

        if( uxBits & BIT_PRESS_SHORT )
        {
            printf("Press short\n");
        }
        else if( uxBits & BIT_PRESS_NORMAL )
        {
            printf("Press normal\n");
        }
        else if( uxBits & BIT_PRESS_LONG)
        {
            printf("Press long\n");
        }
        else
        {
            /* xEventGroupWaitBits() returned because xTicksToWait ticks passed
            without either BIT\_0 or BIT\_4 becoming set. */
        }
       
    }

}

void app_main(void)
{
    xCreatedEventGroup = xEventGroupCreate();
    output_create (BLINK_LED_GPIO);
    input_io_create(BUTTON_GPIO, ANY_EDGE);
    input_set_callback(input_event_callback);
    timeout_button_set_callback( button_timeout_callback);
    xTaskCreate(vTaskCode,
                         "vTaskcode",
                         2048,
                        NULL,
                         4,
                        NULL);
}