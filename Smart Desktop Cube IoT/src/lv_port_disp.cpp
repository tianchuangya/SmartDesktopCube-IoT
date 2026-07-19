#include "lv_port_disp.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

#define TFT_MOSI 12
#define TFT_SCLK 13
#define TFT_CS   16
#define TFT_DC   14
#define TFT_RST  4
#define TFT_BL   42

static Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
static constexpr uint32_t DISP_BUF_SIZE = 320 * 48;
static lv_color_t buf1[DISP_BUF_SIZE];
static lv_color_t buf2[DISP_BUF_SIZE];
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

static void disp_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.writePixels((uint16_t *)color_p, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(drv);
}

void lv_port_disp_init(void)
{
    tft.begin();
    tft.setRotation(1);

    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, 255);   // 使用PWM初始化，确保后续亮度调节生效

    lv_init();

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISP_BUF_SIZE);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}

void lv_port_disp_set_backlight(uint8_t brightness)
{
    if (brightness > 100) brightness = 100;
    analogWrite(TFT_BL, brightness * 255 / 100);
}
