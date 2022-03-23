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

/**
 * @brief LCDモニタ表示クラス
 */
class M5_LCD {
    private:
        // 画面輝度
        const uint8_t brightness[5] = { 0, 5, 20 ,80 ,200 }; //輝度テーブル
        uint8_t light = 4;      // 現在の輝度の保存

        // 温度パネル スプライト
        TFT_eSprite panel1      = TFT_eSprite(&M5.Lcd);
        TFT_eSprite panel2      = TFT_eSprite(&M5.Lcd);
        TFT_eSprite panel3      = TFT_eSprite(&M5.Lcd);
        TFT_eSprite panel4      = TFT_eSprite(&M5.Lcd);
        TFT_eSprite panel5      = TFT_eSprite(&M5.Lcd);
        TFT_eSprite panel6      = TFT_eSprite(&M5.Lcd);
        TFT_eSprite *PN[MAX_SENS] = { &panel1, &panel2, &panel3, &panel4, &panel5, &panel6 };

        // ステータスバー　スプライト
        TFT_eSprite status      = TFT_eSprite(&M5.Lcd);

        // QRコード表示文字列
        String URL;
        String LINE;

        // netRTC(時刻取得用）
        netRTC *RTC;

    public:
        M5_LCD();
        void init();
        void setRTC( netRTC * rtc );
        
        bool update( uint8_t n, SSTAT_t stat, sData * dt );
        void updateMyself( sData *dt );
        
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


    private:
        // Screen Area DATA
        
        const uint16_t LCD_WIDTH      = 320;
        const uint16_t LCD_HEIGHT     = 240;
        const uint16_t LCD_TXT_SIZE   = 7;
        const uint16_t PAN_WIDTH      = 106;
        const uint16_t PAN_HEIGHT     = 111;
        const uint16_t STT_WIDTH      = 320;
        const uint16_t STT_HEIGHT     = 18;
        const uint16_t LCD_TXT_SIZE_ST = 2;

        // Panel Positions
        //const uint8_t  PAN_POS_X0     = 0;
        //const uint8_t  PAN_POS_X1     = 107;
        //const uint8_t  PAN_POS_X2     = 213;
        //const uint8_t  PAN_POS_Y0     = 0;
        //const uint8_t  PAN_POS_Y1     = 111;
        //const uint8_t  STT_POS_X0     = 0;
        //const uint8_t  STT_POS_Y0     = 222;
        bool  bUpdated[MAX_SENS]; 
        const st_POS PN_pos[MAX_SENS] = { { 0,0 }, { 107, 0 }, { 213, 0}, { 0,111 }, { 107, 111 }, { 213, 111} }; 
        const st_POS ST_pos = { 0, 222 };


        //background color
        const uint16_t  BGC_PAN_NORM = M5.Lcd.color565( 0x00, 0x70, 0x33 );
        const uint16_t  BGC_PAN_WARN = M5.Lcd.color565( 0x00, 0x14, 0x62 );
        const uint16_t  BGC_PAN_CAUT = M5.Lcd.color565( 0xf2, 0x00, 0x51 );
        const uint16_t  BGC_STT      = M5.Lcd.color565( 0x00, 0x00, 0x7b );
        const uint16_t  BGC_TRANSP   = M5.Lcd.color565( 0x00, 0xff, 0x00 );


#if 0
        // Graph Area Data
        const uint16_t CHART_WIDTH    = 320;
        const uint16_t CHART_HEIGHT   = 113;
        const uint8_t  CHART_POS_X    = 0;
        const uint8_t  CHART_POS_Y    = 111;
        const uint16_t  BGC_GRAP     = M5.Lcd.color565( 0x00, 0x38, 0x65 );
#endif

};

#endif __M5_DISPLAY_H__
