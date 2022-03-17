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

// イメージデータ配列作成
const uint16_t *PANEL_img[] = { bg_pan_nor, bg_pan_high, bg_pan_high, bg_pan_low, bg_pan_low };
const uint16_t *ANT_img[] = {ANT_1, ANT_2, ANT_3, ANT_4};
const uint16_t *BATT_img[] = {BATT_0, BATT_1, BATT_2, BATT_3};

#endif __SAST_M5_H__
