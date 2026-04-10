idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES ESPNOW_RX
                    REQUIRES radio esp_wifi nvs_flash)
