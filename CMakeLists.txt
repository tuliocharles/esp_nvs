idf_component_register(SRCS "esp_nvs.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES
                        nvs_flash)
