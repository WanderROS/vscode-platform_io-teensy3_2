#include <Arduino.h>
#include "../lib/TFT_ST7735/TFT_ST7735.h"
#include <SPI.h>
#include "../lib/TFT_ST7735/fonts/fluide_caps.h"
#include <lvgl.h>
#include <SoftwareSerial.h>
/*
Teensy3.x and Arduino's
You are using 4 wire SPI here, so:
 MOSI:  11//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 MISO:  12//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 SCK:   13//Teensy3.x/Arduino UNO (for MEGA/DUE refere to arduino site)
 the rest of pin below:
 */
#define __CS 10
#define __DC 6
#define __RST 23
/*
Teensy 3.x can use: 2,6,10,15,20,21,22,23
Arduino's 8 bit: any
DUE: check arduino site
If you do not use reset, tie it to +3V3
*/

TFT_ST7735 tft = TFT_ST7735(__CS, __DC, __RST);
static const uint32_t screenWidth = 128;
static const uint32_t screenHeight = 160;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  int32_t x, y;
  for (y = area->y1; y <= area->y2; y++)
  {
    for (x = area->x1; x <= area->x2; x++)
    {
      tft.drawPixel(x, y, *(uint16_t *)color_p);
      color_p++;
    }
  }
  lv_disp_flush_ready(disp);
}

static void event_cb(lv_event_t *e)
{
  LV_LOG_USER("Clicked");

  static uint32_t cnt = 1;
  lv_obj_t *btn = lv_event_get_target(e);
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  lv_label_set_text_fmt(label, "%" LV_PRIu32, cnt);
  cnt++;
}

/**
 * Add click event to a button
 */
void lv_example_event_1(void)
{
  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn, 70, 60);
  lv_obj_center(btn);
  lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Click me!");
  lv_obj_center(label);
}

void setup()
{
  tft.begin();
  tft.setRotation(2);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  lv_example_event_1();

  // 调试用的串口
  Serial.begin(9600);
  Serial.println("默认串口初始化！");
  Serial1.setRX(21);
  Serial1.setTX(5);
  Serial1.begin(9600);
}
String inputString = "test";
void loop()
{
  // put your main code here, to run repeatedly:
  lv_timer_handler(); /* let the GUI do its work */
  delay(1);
  // Serial1.println("串口1初始化！");
  while (Serial1.available())
  {
    // get the new byte:
    char inChar = (char)Serial1.read();
    Serial.print(inChar);
  }
}

void serialEvent()
{
  while (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    Serial1.print(inChar);
  }
}
