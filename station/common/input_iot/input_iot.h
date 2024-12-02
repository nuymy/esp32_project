#ifndef __INPUT_DEV_H
#define __INPUT_DEV_H
#include <stdio.h>
#include <driver/gpio.h>
#include <hal/gpio_types.h>
#include <freertos/FreeRTOS.h>

#define BUTTON_GPIO GPIO_NUM_0

typedef enum {
    LO_TO_HI = 1,
    HI_TO_LO = 2,
    ANY_EDGE = 3 
}   interrupt_type_edge_t;

typedef void (*input_callback_t) (int pin, uint64_t tick);
typedef void (*timeoutButton_t)(int);
void input_io_create (gpio_num_t gpio_num, interrupt_type_edge_t type);

void intput_io_get_level(gpio_num_t gpio_num);
void input_set_callback(void *cb);

void timeout_button_set_callback(void *cb);

#endif