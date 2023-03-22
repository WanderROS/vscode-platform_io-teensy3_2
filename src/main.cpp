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
// WIFI wifi = WIFI();
void setup()
{
  tft.begin();
  tft.setRotation(2);

  // lv_init();
  // lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  // /*Initialize the display*/
  // static lv_disp_drv_t disp_drv;
  // lv_disp_drv_init(&disp_drv);
  // /*Change the following line to your display resolution*/
  // disp_drv.hor_res = screenWidth;
  // disp_drv.ver_res = screenHeight;
  // disp_drv.flush_cb = my_disp_flush;
  // disp_drv.draw_buf = &draw_buf;
  // lv_disp_drv_register(&disp_drv);

  // lv_example_event_1();
  // tft.print("receive:");
  // 调试用的串口
  Serial.begin(9600);
  Serial.println("默认串口初始化！");
  Serial1.setRX(21);
  Serial1.setTX(5);
  Serial1.begin(9600);
}
extern void processWiFiRecvOrders(char c);
extern void processDeviceRecvOrders(char c);
void loop()
{
  // put your main code here, to run repeatedly:
  // lv_timer_handler(); /* let the GUI do its work */
  delay(1);

  // Serial1.println("串口1初始化！");
  while (Serial1.available())
  {
    // get the new byte:
    char inChar = (char)Serial1.read();
    // Serial.println("默认串口初始化！");
    processWiFiRecvOrders(inChar);
    // Serial.print(inChar);
    //  wifi.processOrders(inChar);
  }
}

void serialEvent()
{
  while (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    processDeviceRecvOrders(inChar);
  }
}

enum Recv_Status
{
  START = 0,
  RECVING,
  END
};

/**
 * CRC 校验
 */
unsigned char orderCheckSum(char *buffer, int len)
{
  unsigned char checksum = 0;
  for (int i = 0; i < len - 2; i++)
  {
    checksum += buffer[1 + i];
  }
  checksum = ~checksum + 1;
  return checksum;
}
// 项目号
uint16_t projectNo = 13104;

void processA0(char *buffer, int len, uint16_t projectNo)
{
  buffer[12] = projectNo & 0xFF;
  buffer[13] = projectNo / 255;
  buffer[len] = orderCheckSum(buffer, len);
}
void process04(char *buffer, int len, uint16_t projectNo)
{
  buffer[31] = projectNo & 0xFF;
  buffer[32] = projectNo / 255;
  buffer[len] = orderCheckSum(buffer, len);
}
void process03(char *buffer, int len, uint16_t projectNo)
{
  buffer[31] = projectNo & 0xFF;
  buffer[32] = projectNo / 255;
  buffer[len] = orderCheckSum(buffer, len);
}
// 注入内容
void injectOrder(char *buffer, int len)
{
  // 设备指令小于 10 没必要注入
  if (len >= 10)
  {
    // 指令
    switch ((unsigned char)buffer[9])
    {
    case 0xA0:
      processA0(buffer, len, projectNo);
      break;
    case 0x04:
      process04(buffer, len, projectNo);
      break;
    case 0x03:
      process03(buffer, len, projectNo);
      break;
    default:
      break;
    }
  }
}
// 处理设备指令
// 串口设备接收缓冲区
char *device_recv_buffer;
// 设备接收状态
Recv_Status device_status;
// 设备帧接收长度
unsigned char device_recv_length;
unsigned char device_recv_cur_index;
void processDeviceRecvOrders(char c)
{
  tft.setCursor(0, 0);
  tft.println("device:");
  int16_t x = -1, y = -1;
  int16_t &x_ali = x;
  int16_t &y_ali = y;
  tft.getCursor(x_ali, y_ali);
  tft.print(" ");
  tft.setCursor(x, y);
  tft.println(device_status);

  if (device_status == Recv_Status::RECVING)
  {
    device_recv_cur_index++;
    device_recv_buffer[device_recv_cur_index] = c;
    if (device_recv_cur_index == device_recv_length - 1)
    {
      device_status = Recv_Status::END;
    }
    tft.getCursor(x_ali, y_ali);
    tft.print("  ");
    tft.setCursor(x, y);
    tft.println(device_recv_cur_index);
    tft.getCursor(x_ali, y_ali);
    tft.print("  ");
    tft.setCursor(x, y);
    tft.println(device_recv_length);
    if (device_status == Recv_Status::END)
    {
      unsigned char checkSum = orderCheckSum(device_recv_buffer, device_recv_length);
      if (checkSum == device_recv_buffer[device_recv_length - 1])
      {
        injectOrder(device_recv_buffer, device_recv_length - 1);
        for (int i = 0; i < device_recv_length; ++i)
        {
          // 中继设备的数据出去
          Serial1.print(device_recv_buffer[i]);
          tft.setCursor(i * 16 % 128, 48 + i * 16 / 128 * 8);
          if (device_recv_buffer[i] > 0xf)
          {
            tft.print(device_recv_buffer[i], 16);
          }
          else
          {
            tft.print('0');
            tft.print(device_recv_buffer[i], 16);
          }
        }
        tft.setCursor(0, 32);
        tft.println("end");
        tft.println(checkSum, 16);
      }
      // tft.setCursor(39, 30);
      // tft.println("receive:" + wifi_recv_buffer[1]);

      delete device_recv_buffer;
      device_recv_cur_index = 0;
      device_recv_length = 0;
    }
  }

  // 收到了0xAA,并且接收数组为空，说明是帧长度字节
  if (device_status == Recv_Status::START)
  {
    device_recv_buffer = new char[c + 1];
    device_status = Recv_Status::RECVING;
    device_recv_length = c + 1;
    device_recv_buffer[0] = 0xAA;
    device_recv_buffer[1] = c;
    device_recv_cur_index = 1;
  }
  // 收到了 MSmart 帧头;
  if (0xAA == c)
  {
    device_status = Recv_Status::START;
    tft.clearScreen();
  }
}

// 接收模组指令
// 串口模组接收缓冲区
char *wifi_recv_buffer;
// 模组接收状态
Recv_Status status;
// 模组帧接收长度
unsigned char recv_length;
unsigned char recv_cur_index;
void processWiFiRecvOrders(char c)
{
  //  tft.setCursor(0, 0);
  //  tft.println("status:");
  int16_t x = -1, y = -1;
  int16_t &x_ali = x;
  int16_t &y_ali = y;
  // tft.getCursor(x_ali, y_ali);
  // tft.print(" ");
  // tft.setCursor(x, y);
  // tft.println(status);

  if (status == Recv_Status::RECVING)
  {
    recv_cur_index++;
    wifi_recv_buffer[recv_cur_index] = c;
    if (recv_cur_index == recv_length - 1)
    {
      status = Recv_Status::END;
    }
    // tft.getCursor(x_ali, y_ali);
    // tft.print("  ");
    // tft.setCursor(x, y);
    // tft.println(recv_cur_index);
    // tft.getCursor(x_ali, y_ali);
    // tft.print("  ");
    // tft.setCursor(x, y);
    // tft.println(recv_length);
    if (status == Recv_Status::END)
    {
      unsigned char checkSum = orderCheckSum(wifi_recv_buffer, recv_length);
      if (checkSum == wifi_recv_buffer[recv_length - 1])
      {
        for (int i = 0; i < recv_length; ++i)
        {
          Serial.print(wifi_recv_buffer[i]);
          // tft.setCursor(i * 16 % 128, 48 + i * 16 / 128 * 8);
          // if (wifi_recv_buffer[i] > 0xf)
          // {
          //   tft.print(wifi_recv_buffer[i], 16);
          // }
          // else
          // {
          //   tft.print('0');
          //   tft.print(wifi_recv_buffer[i], 16);
          // }
        }
        // tft.setCursor(0, 32);
        // tft.println("end");
        // tft.println(checkSum, 16);
      }
      // tft.setCursor(39, 30);
      // tft.println("receive:" + wifi_recv_buffer[1]);

      delete wifi_recv_buffer;
      recv_cur_index = 0;
      recv_length = 0;
    }
  }

  // 收到了0xAA,并且接收数组为空，说明是帧长度字节
  if (status == Recv_Status::START)
  {
    wifi_recv_buffer = new char[c + 1];
    status = Recv_Status::RECVING;
    recv_length = c + 1;
    wifi_recv_buffer[0] = 0xAA;
    wifi_recv_buffer[1] = c;
    recv_cur_index = 1;
  }
  // 收到了 WB01 模组的帧头;
  if (0xAA == c)
  {
    status = Recv_Status::START;
    // tft.clearScreen();
  }
}