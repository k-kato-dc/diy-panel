add_executable(diy-pico
        main.c
)

# exit main() to reboot with BOOTSEL
target_compile_options(diy-pico INTERFACE -DPICO_ENTER_USB_BOOT_ON_EXIT=1)
##target_compile_options(diy-pico INTERFACE -DPICO_ENTER_USB_BOOT_ON_EXIT=0)

# pull in common dependencies
target_link_libraries(diy-pico LCD pico_stdlib hardware_flash hardware_sync)

# enable usb, disable uart
pico_enable_stdio_usb(diy-pico 1)
pico_enable_stdio_uart(diy-pico 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(diy-pico)

# call pico_set_program_url to set path to example on github, so users can find the source for an example via picotool
##example_auto_set_url(diy-pico)
