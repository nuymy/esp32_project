#ifndef __LEDC_APP_
#define __LEDC_APP_

void ledc_init(void);
void ledc_add_pin(int pin, int channel);
void ledc_app_set_duty(int channel, int duty);

#endif