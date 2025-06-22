aux_source_directory(. DIR_LCD_SRCS)
add_library(LCD ${DIR_LCD_SRCS})

target_link_libraries(LCD pico_stdlib hardware_spi hardware_pwm)
