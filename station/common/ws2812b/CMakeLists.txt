set(app_src ws2812b.c)
set(pri_req led_strip esp_driver_rmt driver)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})

