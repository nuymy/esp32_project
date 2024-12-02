idf_component_register(SRCS "hello_world_main.c"
                    PRIV_REQUIRES spi_flash
                    PRIV_REQUIRES output_iot 
                    PRIV_REQUIRES input_iot )
