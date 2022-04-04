/**
 * @file SAST_M5.h
 * @author Fumihito Takahashi (fumihito.takahashi@sem-it.com)
 * @brief SAST M5 用 共通データ構造
 * @version 0.1
 * @date 2022-03-07
 * 
 * @copyright Copyright (c) SEM-IT 2022
 * 
 */
#ifndef __SAST_M5_H__
#define __SAST_M5_H__

#ifdef SAST_DEBUG
#define PRT_MEMORY  { Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DMA):%d\n", heap_caps_get_free_size(MALLOC_CAP_DMA) );  Serial.printf("heap_caps_get_largest_free_block(MALLOC_CAP_DMA):%d\n", heap_caps_get_largest_free_block(MALLOC_CAP_DMA) );}
#else
#define PRT_MEMORY  
#endif

#include <M5Stack.h>
#include "image.h"      // 画面背景データ
#include "time.h"
#include <WiFi.h>
#include <Ticker.h>


/**
 * @brief Sensor Type
 */
enum class  SENS_t : int {
    None = 0,
    Lazurite,
    TH1,
};

/**
 * @brief Sensor Status
 * 
 */
enum class SSTAT_t : int {
  normal =0,
  warn,
  caution,
  disable,
};


/**
 * @brief グラフパネルのモード（イメージ選択）
 */
enum class PANEL_t : int {
  NORM = 0,
  WARN_H,
  CAUT_H,
  WARN_L,
  CAUT_L,
};

/**
 * @brief グラフモード
 */
enum class GRAPH_t {
  TEMPL = 0,
  HUMID,
  PRESS,
  AVS,
};

/**
 * @brief 座標構造体
 */
typedef struct {
  uint8_t x;
  uint8_t y;
}st_POS;


/**
 * @brief Ambientデータ構造体
 */
typedef struct {
  float  dt;
  bool   use = false;
}st_AMB;


// 最大センサー数
const uint8_t MAX_SENS = 6;

// 画面サイズ、フォントベースサイズ指定
const uint16_t LCD_WIDTH      = 320;
const uint16_t LCD_HEIGHT     = 240;
const uint16_t LCD_TXT_SIZE   = 7;

//background color
const uint16_t  BGC_PAN_NOR =  M5.Lcd.color565( 0x00, 0x70, 0x33 );
const uint16_t  BGC_PAN_LOW =  M5.Lcd.color565( 0x00, 0x14, 0x62 );
const uint16_t  BGC_PAN_HIG = M5.Lcd.color565( 0xf2, 0x00, 0x51 );
const uint16_t  BGC_GRAP    = M5.Lcd.color565( 0x00, 0x38, 0x65 );
const uint16_t  BGC_STAT    = M5.Lcd.color565( 0x00, 0x00, 0x7b );

#endif __SAST_M5_H__
