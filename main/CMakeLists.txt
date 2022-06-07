idf_component_register(
    SRCS 
        main.cpp
        app_display.cpp
        openhaystack.c
    INCLUDE_DIRS
        .
        include
    REQUIRES driver esp_adc_cal LovyanGFX lvgl esp32-button protocol_examples_common bt

)
