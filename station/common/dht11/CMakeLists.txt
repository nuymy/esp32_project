set (app_src dht11.c)
set (pri_req esp_timer esp_driver_gpio)

idf_component_register(SRCS ${app_src}
                        INCLUDE_DIRS "."
                        REQUIRES ${pri_req}) 
