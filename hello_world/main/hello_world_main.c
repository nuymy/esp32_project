/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h" 
#include "freertos/timers.h" 
#include "freertos/event_groups.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "output_dev.h"
#include "input_iot.h"
#include <freertos/portmacro.h>
#include "driver/gpio.h"

#define BIT_EVENT_BUTTON_PRESS      (1<<0)
#define BIT_EVENT_UART_RECV        (1<<1)

 
TimerHandle_t xTimers[2];
EventGroupHandle_t xEventGroup;

void vTask1 (void *pvParameters)
{
    for (;;)
    {
        EventBits_t uxBits =  xEventGroupWaitBits(
                        xEventGroup,
                      BIT_EVENT_BUTTON_PRESS | BIT_EVENT_UART_RECV,
                      pdTRUE,
                      pdFALSE,
                      portMAX_DELAY );

        if (uxBits & BIT_EVENT_BUTTON_PRESS)
        {
            printf("Button pressed\n");
            output_toggle(2);
        }

        if (uxBits & BIT_EVENT_UART_RECV)
        {
            printf("Received data\n");
        }

    }
}

void vTimerCallback(TimerHandle_t xTimer)
{
    configASSERT(xTimer); 
    int ulCount = (uint32_t)pvTimerGetTimerID(xTimer);
    if (ulCount == 0)
    {
        output_toggle(2);

    }
    else if (ulCount == 1)
    {
        printf("Hello\n");
    }
}


// void vTask2 (void *pvParameters)
// {
//     for (;;) 
//     {
//         printf("Task 2 executing\n");
//         vTaskDelay(1000 / portTICK_PERIOD_MS);

//     }
// }

// void vTask3 (void *pvParameters)
// {
//     for (;;) 
//     {
//         printf("Task 3 executing\n");
//         vTaskDelay(1000 / portTICK_PERIOD_MS);

//     }
// }

void button_callback (int pin)
{
    if (pin == GPIO_NUM_0)
    {
        BaseType_t pxHigherPriorityTaskWoken;
        xEventGroupSetBitsFromISR( xEventGroup,BIT_EVENT_BUTTON_PRESS, &pxHigherPriorityTaskWoken);
    }
}


void app_main(void)
{

    // xTaskCreate( vTask2,"vTask2",1024, NULL, 5, NULL );
    // xTaskCreate( vTask3,"vTask3",1024, NULL, 6, NULL );

    

    //  xTimers[0] =  xTimerCreate( "TimerBlink",pdMS_TO_TICKS(500),pdTRUE,(void*) 0,vTimerCallback );
    //  xTimers[1] =  xTimerCreate( "TimerPrint",pdMS_TO_TICKS(1000),pdTRUE,(void*) 1,vTimerCallback );
     
     output_create( 2);
     input_io_create(0, HI_TO_LO);
     input_set_callback(button_callback);

    // xTimerStart( xTimers[0], 0 );
    // xTimerStart( xTimers[1], 0 );

     xTaskCreate( vTask1,"vTask1",1024*2, NULL, 4, NULL );

}
