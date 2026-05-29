#include "lv_port_touch.h"
#include <Arduino.h>
#include <SPI.h>

#define TOUCH_SCLK 18
#define TOUCH_MISO 20
#define TOUCH_MOSI 17
#define TOUCH_CS   47
#define TOUCH_IRQ  21

static uint16_t xpt2046_read(uint8_t cmd)
{
    digitalWrite(TOUCH_CS, LOW);
    SPI.transfer(cmd);
    uint8_t hi = SPI.transfer(0x00);
    uint8_t lo = SPI.transfer(0x00);
    digitalWrite(TOUCH_CS, HIGH);
    return ((uint16_t)hi << 8 | lo) >> 4;
}

static bool xpt2046_get_xy(int16_t *x, int16_t *y)
{
    int16_t z1 = xpt2046_read(0xB0);
    if (z1 < 20 || z1 > 4000) return false;

    int16_t raw_x = xpt2046_read(0x90);
    int16_t raw_y = xpt2046_read(0xD0);

    if (raw_x < 80 || raw_x > 4000 || raw_y < 80 || raw_y > 4000) return false;

    *x = map(raw_x, 150, 2000, 0, 320);
    *y = map(raw_y, 150, 2000, 0, 240);

    if (*x < 0) *x = 0;
    if (*x >= 320) *x = 319;
    if (*y < 0) *y = 0;
    if (*y >= 240) *y = 239;

    return true;
}

static void my_touch_read(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
    int16_t x, y;
    if (xpt2046_get_xy(&x, &y)) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void lv_port_touch_init(void)
{
    SPI.begin(TOUCH_SCLK, TOUCH_MISO, TOUCH_MOSI);
    SPI.setFrequency(2000000);
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read;
    lv_indev_drv_register(&indev_drv);
}
