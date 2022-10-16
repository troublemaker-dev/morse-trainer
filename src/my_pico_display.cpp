#include <string.h>
#include <math.h>
#include <vector>
#include <cstdlib>

#include "pico_display.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "rgbled.hpp"
#include "button.hpp"

using namespace pimoroni;

ST7789 st7789(PicoDisplay::WIDTH, PicoDisplay::HEIGHT, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));
PicoGraphics_PenRGB332 graphics(st7789.width, st7789.height, nullptr);

RGBLED led(PicoDisplay::LED_R, PicoDisplay::LED_G, PicoDisplay::LED_B);

Button button_a(PicoDisplay::A);
Button button_b(PicoDisplay::B);
Button button_x(PicoDisplay::X);
Button button_y(PicoDisplay::Y);

// HSV Conversion expects float inputs in the range of 0.00-1.00 for each channel
// Outputs are rgb in the range 0-255 for each channel
void from_hsv(float hue, float saturation, float value, uint8_t &r, uint8_t &g, uint8_t &b) {
    float i = floor(hue * 6.0f);
    float f = hue * 6.0f - i;
    value *= 255.0f;
    uint8_t p = value * (1.0f - saturation);
    uint8_t q = value * (1.0f - f * saturation);
    uint8_t t = value * (1.0f - (1.0f - f) * saturation);

    switch (int(i) % 6) {
        case 0: r = value; g = t; b = p; break;
        case 1: r = q; g = value; b = p; break;
        case 2: r = p; g = value; b = t; break;
        case 3: r = p; g = q; b = value; break;
        case 4: r = t; g = p; b = value; break;
        case 5: r = value; g = p; b = q; break;
    }
}

const uint32_t RAYS = 15;
const uint32_t RAY_WIDTH = 10;

void draw_pinwheel(uint32_t index, uint32_t rays = RAYS, uint32_t ray_width = RAY_WIDTH) {
    uint8_t red = 0, green = 0, blue = 0;

    for (int r = 0; r < rays; r++) {
        for (int rw = 0; rw < ray_width; rw++) {
            float rads = ((M_PI * 2.0f) / 15.0f) * float(r);
            rads += (float(index) / 100.0f);
            rads += (float(rw) / 100.0f);
            float cx = sin(rads) * 300.0f;
            float cy = cos(rads) * 300.0f;
            from_hsv(rads, 1.0f, 0.5f + sinf(rads / 100.0f / M_PI) * 0.5f, red, green, blue);
            graphics.set_pen(red, green, blue);
            graphics.line(Point(120, 67), Point(cx + 120, cy + 67));
        }
    }
}

int main() {
    st7789.set_backlight(100);

    struct pt {
        float x;
        float y;
        uint8_t r;
        float dx;
        float dy;
        uint16_t pen;
    };

    // 100 balls to bounce around
    std::vector<pt> shapes;
    for (int i = 0; i < 100; i++) {
        pt shape;
        shape.x = rand() % 240;
        shape.y = rand() % 135;
        shape.r = (rand() % 10) + 3;
        shape.dx = float(rand() % 255) / 128.0f;
        shape.dy = float(rand() % 255) / 128.0f;
        shape.pen = graphics.create_pen(rand() % 255, rand() % 255, rand() % 255);
        shapes.push_back(shape);
    }

    // an index
    uint32_t index = 0;
    uint32_t direction = 1;

    // various pens
    Pen BG = graphics.create_pen(120, 40, 60);
    Pen YELLOW = graphics.create_pen(255, 255, 0);
    Pen TEAL = graphics.create_pen(0, 255, 255);
    Pen WHITE = graphics.create_pen(255, 255, 255);

    while (true) {
        //test buttons
        if(button_a.raw()) direction = 1;
        if(button_b.raw()) direction = -1;

//        if(button_x.raw()) text_location.y -= 1;
//        if(button_y.raw()) text_location.y += 1;

        // set background
        graphics.set_pen(BG);
        graphics.clear();

        // move all the balls
        for (auto &shape: shapes) {
            shape.x += shape.dx;
            shape.y += shape.dy;
            if (shape.x < 0) shape.dx *= -1;
            if (shape.x >= graphics.bounds.w) shape.dx *= -1;
            if (shape.y < 0) shape.dy *= -1;
            if (shape.y >= graphics.bounds.h) shape.dy *= -1;

            graphics.set_pen(shape.pen);
            graphics.circle(Point(shape.x, shape.y), shape.r);
        }

        // cycle RGB LED dim to bright
//        float led_step = fmod(index / 20.0f, M_PI * 2.0f);
//        int r = (sin(led_step) * 32.0f) + 32.0f;
//        led.set_rgb(r, r / 1.2f, r);

        // Since HSV takes a float from 0.0 to 1.0 indicating hue,
        // then we can divide millis by the number of milliseconds
        // we want a full colour cycle to take. 5000 = 5 sec.
        uint8_t r = 0, g = 0, b = 0;
        from_hsv((float)millis() / 5000.0f, 1.0f, 0.5f + sinf(millis() / 100.0f / M_PI) * 0.5f, r, g, b);
        led.set_rgb(r, g, b);

        // create one poly
        std::vector<Point> poly;
        poly.push_back(Point(30, 30));
        poly.push_back(Point(50, 35));
        poly.push_back(Point(70, 25));
        poly.push_back(Point(80, 65));
        poly.push_back(Point(50, 85));
        poly.push_back(Point(30, 45));

        graphics.set_pen(YELLOW);
        //pico_display.pixel(Point(0, 0));
        graphics.polygon(poly);

        // create one triangle
        graphics.set_pen(TEAL);
        graphics.triangle(Point(50, 50), Point(130, 80), Point(80, 110));

        // draw three lines between
        graphics.set_pen(WHITE);
        graphics.line(Point(50, 50), Point(120, 80));
        graphics.line(Point(20, 20), Point(120, 20));
        graphics.line(Point(20, 20), Point(20, 120));

        // draw a pinwheel with 15 rays, each 10 units wide
        draw_pinwheel(index, 15, 10);

        // update screen
        st7789.update(&graphics);
        sleep_ms(1000 / 60);
        index += direction;
    }

    return 0;
}
