cmake_minimum_required(VERSION 3.8.2)
# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory.
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(BOARD nrf52840_pca10056)
add_definitions(-DNRFX_PDM_ENABLED=1)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

# Add all our sources
FILE(GLOB app_sources src/*.c src/mesh/*.c src/bling/*.c src/drivers/ff_i2c.c)
target_sources(app PRIVATE ${app_sources})

# Conditionally add the drivers
zephyr_library_sources_ifdef(CONFIG_AT24C02 src/drivers/at24c02.c)
zephyr_library_sources_ifdef(CONFIG_IQS333 src/drivers/iqs333.c)
zephyr_library_sources_ifdef(CONFIG_IS31FL3741	src/drivers/is31fl3741.c)
zephyr_library_sources_ifdef(CONFIG_FANCYFEAST_USB_MASS_STORAGE	src/drivers/usb_msc.c)
zephyr_library_sources_ifdef(CONFIG_SPI_FLASH_W25QXXDV src/drivers/spi_nor_reset.c)

#Manually add in the PDM driver since Zephyr doesn't quite support this
#zephyr_sources_ifdef(CONFIG_NRFX_PDM    $ENV{ZEPHYR_BASE}/ext/hal/nordic/nrfx/drivers/src/nrfx_pdm.c)

# Add custom target to generate mcuboot image
add_custom_target(sign_bin ../../mcuboot/scripts/imgtool.py sign -k ../../mcuboot/root-rsa-2048.pem --align 8 -H 0x200 --version 23 -S 0xcc000 zephyr/zephyr.bin andnxor_dc27_signed.bin)
add_custom_target(sign arm-none-eabi-objcopy --change-address 0xd000 -I binary -O ihex andnxor_dc27_signed.bin andnxor_dc27_signed.hex)
add_dependencies(sign sign_bin)

# Add custom targets to flash bootloader and signed image
add_custom_target(flash_bootloader nrfjprog --sectorerase --program ../../../provision/fancyfeast_bootloader.hex -f nrf52)
add_custom_target(flash_signed nrfjprog --sectorerase --program andnxor_dc27_signed.hex -f nrf52 -r)
add_custom_target(flash_ocd_signed ../../../tools/openocd/ocd-flash.sh andnxor_dc27_signed.hex)
add_custom_target(kanye nrfjprog --sectorerase --program zephyr/zephyr.hex -f nrf52 -r)
add_custom_target(gdb_server JLinkGDBServer -device NRF52 -if SWD -speed 1000)
add_custom_target(gdb echo "RUN THIS:     arm-none-eabi-gdb -ex "target remote localhost:2331" --se=zephyr/zephyr.elf")