add_executable(
  ${NAME}
  my_pico_display.cpp
  image_data.cpp
)

include(common/pimoroni_bus)
include(drivers/button/button)
include(drivers/st7789/st7789)
include(drivers/rgbled/rgbled)
include(libraries/bitmap_fonts/bitmap_fonts)
include(libraries/hershey_fonts/hershey_fonts)
include(libraries/pico_display/pico_display)
include(libraries/pico_graphics/pico_graphics)


# Pull in pico libraries that we need
target_link_libraries(${NAME} pico_stdlib hardware_spi hardware_pwm hardware_pio hardware_dma rgbled button pico_display pico_graphics st7789)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})