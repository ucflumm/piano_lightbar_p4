#pragma once

#define PANEL_H_RES 480
#define PANEL_V_RES 800
#define LVGL_H_RES 800
#define LVGL_V_RES 480
#define LCD_DRAW_BUF_LINES 20

#define MIPI_DSI_LANE_NUM 2
#define MIPI_DSI_LANE_BITRATE_MBPS 500

#define LCD_DPI_CLK_MHZ 34
#define LCD_HSYNC 12
#define LCD_HBP 42
#define LCD_HFP 42
#define LCD_VSYNC 2
#define LCD_VBP 8
#define LCD_VFP 166

#define PIN_LCD_RST 5

#define PIN_LCD_BK_LIGHT 23
#define LCD_BK_LIGHT_ON_LEVEL 1
#define LCD_BK_LIGHT_OFF_LEVEL 0

// User-confirmed board-level mapping:
//   Touch INT  -> GPIO21
//   Touch RST  -> GPIO22
//   LCD PWM    -> GPIO23
//   Touch I2C  -> SDA GPIO7, SCL GPIO8
// Note: These are ESP32 GPIO numbers on the board connector, not GT911 IC pin numbers.
#define PIN_TOUCH_I2C_SDA 7
#define PIN_TOUCH_I2C_SCL 8
#define PIN_TOUCH_INT 21
#define PIN_TOUCH_RST 22
#define TOUCH_I2C_PORT 0
#define TOUCH_I2C_CLK_HZ 400000

#define MIPI_DSI_PHY_PWR_LDO_CHAN 3
#define MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500
