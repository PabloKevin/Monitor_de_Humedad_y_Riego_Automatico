idf_component_register(
    SRCS "Lab3_main.cpp" "LCD.cpp"
    INCLUDE_DIRS ""
    REQUIRES vfs
    PRIV_REQUIRES esp_adc driver freertos
)
