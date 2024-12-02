# idf_component_register(SRCS "app_main.c" "app_config.c"
#                     INCLUDE_DIRS ".")
set(app_src app_main.c app_config.c)
set(pri_req esp_wifi nvs_flash)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})
                    