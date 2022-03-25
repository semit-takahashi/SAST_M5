/**
 * @file display.h
 * @author Fumihito Takahashi (fumihito.takahashi@sem-it.com)
 * @brief 
 * @version 0.1
 * @date 2022-03-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __M5_DISPLAY_H__
#define __M5_DISPLAY_H__

#include "SAST_M5.h"
#include "netRTC.h"
#include "sensor.h"

//Protype
class sData;
class SensList;
class netRTC;

/**
 * @brief LCDモニタ表示クラス
 */
class M5_LCD {
    private:
        // 画面輝度
        const uint8_t brightness[5] = { 0, 5, 20 ,80 ,200 }; //輝度テーブル
        uint8_t light = 4;      // 現在の輝度の保存

        // QRコード表示文字列
        String URL;
        String LINE;

        // netRTC(時刻取得用）
        netRTC *RTC;

        // SesnsList(データ取得用)
        SensList *SENS;

    public:
        void init( netRTC* rtc, SensList *sns ) ;
        bool update( uint16_t n, SSTAT_t stat, sData *dt );
        
        void showURL();
        void showLINE();
        void setLINE( String url );
        void setURL( String url );
        void setBrightness( int8_t brt = -1 );
        
        void clear();
        void draw( bool all = false );
        void reDraw();

    private:
        void showQR( String url, String caption = "" );
        void drawPanel( bool all = false );
        void drawStatusBar();
        uint8_t ftoa1( float val );
        void makePanelBG( TFT_eSprite *sp, SSTAT_t stat );
        


    private:
        // Screen Area DATA
        
        const uint16_t LCD_WIDTH      = 320;
        const uint16_t LCD_HEIGHT     = 240;
        const uint16_t LCD_TXT_SIZE   = 7;

        const uint16_t PAN_WIDTH      = 106;
        const uint16_t PAN_HEIGHT     = 111;

        const uint16_t STT_WIDTH      = 320;
        const uint16_t STT_WIDTH_TP   = 242;
        const uint16_t STT_WIDTH_TM   = 78;
        const uint16_t STT_HEIGHT     = 18;
        const uint16_t STT_TXT_Y      = 2;
        const uint16_t STT_TXT_SIZE   = 1;


        const st_POS PN_pos[MAX_SENS] = { { 0,0 }, { 107, 0 }, { 213, 0}, { 0,111 }, { 107, 111 }, { 213, 111} }; 
        const st_POS ST_pos = { 0, 222 };
        const st_POS TT_pos = { 242, 222 };


        //background color
        const uint16_t  BGC_PAN_NORM = M5.Lcd.color565( 0x00, 0x70, 0x33 );
        const uint16_t  BGC_PAN_WARN = M5.Lcd.color565( 0xff, 0x8a, 0x00 );
        const uint16_t  BGC_PAN_CAUT = M5.Lcd.color565( 0xf2, 0x00, 0x51 );
        const uint16_t  BGC_PAN_NONE = 0x7BEF;  // TFT_DARKGRAY
        const uint16_t  BGC_STT      = M5.Lcd.color565( 0x00, 0x00, 0x7b );
        const uint16_t  BGC_TRANSP   = M5.Lcd.color565( 0x00, 0xff, 0x00 );
        const uint16_t  BGC_LINE_D   = TFT_DARKGREY;
        const uint16_t  BGC_LINE_L   = TFT_LIGHTGREY;
        const uint16_t  PANEL_bgc[4] = {BGC_PAN_NORM, BGC_PAN_WARN, BGC_PAN_CAUT, BGC_PAN_NONE };

};

#endif __M5_DISPLAY_H__
