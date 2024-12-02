#include "input_iot.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <stdio.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/portmacro.h>
// #include <freertos/FreeRTOSConfig.h>
// #include <sdkconfig.h>
// #include <portmacro.h>


input_callback_t input_callback = NULL;
timeoutButton_t timeoutButton_callback = NULL;

TimerHandle_t xTimers;

static uint64_t start_time, stop_time, press_tick;

static void gpio_input_handler (void *arg)
{
    int gpio_num = (uint32_t)arg;
    // input_callback(gpio_num);
    uint32_t rtc = xTaskGetTickCountFromISR(); // get time from isr interrupt

    if (gpio_get_level (gpio_num) == 0)
    {
        xTimerStart(xTimers, 0);
        start_time = rtc;
    }
    else
    {
        xTimerStop(xTimers, 0);
        stop_time = rtc;
        press_tick = stop_time - start_time;
        input_callback(gpio_num, press_tick);
    }
}

static void vTimerCallback( TimerHandle_t xTimer )
 {
    uint32_t ID;

    /* Optionally do something if the pxTimer parameter is NULL. */
    configASSERT( xTimer );

    /* The number of times this timer has expired is saved as the
       timer's ID. Obtain the count. */
    ID = ( uint32_t ) pvTimerGetTimerID( xTimer );

    /* Increment the count, then test to see if the timer has expired
       ulMaxExpiryCountBeforeStopping yet. */
    if (ID == 0)
    {
        timeoutButton_callback(BUTTON_GPIO);
    }
 }

void input_io_create (gpio_num_t gpio_num, interrupt_type_edge_t type)
{
    esp_rom_gpio_pad_select_gpio(gpio_num);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);
    gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(gpio_num, type);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_num, gpio_input_handler, (void*)gpio_num);


    xTimers = xTimerCreate
                 ( "TimerForTimeout",
                   5000/portTICK_PERIOD_MS,
                   pdFALSE,
                   (void*)0,
                   vTimerCallback );
}

void input_set_callback(void *cb)
{
    input_callback = cb;
}

void timeout_button_set_callback(void *cb)
{
    timeoutButton_callback = cb;
}