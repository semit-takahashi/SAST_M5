/**
 * @file display.cpp
 * @author Fumihito Takahashi (fumihito.takahashi@sem-it.com)
 * @brief M5Stackモニタ部分
 * @version 0.1
 * @date 2022-03-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "display.h"

// イメージデータ配列作成
//const uint16_t *PANEL_img[] = { bg_pan_nor, bg_pan_warn, bg_pan_caut};
const uint16_t *ANT_img[] = {ANT_1, ANT_2, ANT_3, ANT_4};
const uint16_t *BATT_img[] = {BATT_0, BATT_1, BATT_2, BATT_3};


/**
 * @brief LCD初期化
 */
void M5_LCD::init( netRTC* rtc, SensList *sns ){
    // LCD初期化
    M5.Lcd.setTextFont(2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.fillScreen(TFT_NAVY);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_NAVY );
    M5.Lcd.setCursor(2, 2);
    setBrightness( 3 );         // 画面輝度初期
    RTC = rtc;
    SENS = sns;
}

/**
 * @brief センサー情報の更新してパネル情報を内部作成
 * @param n センサーNO（0〜5）
 * @param stat センサーステータス(NORMALWARN,CAUTION) 
 * @param dt センサデータ
 */
bool M5_LCD::update( uint16_t n, SSTAT_t stat, sData *dt ){
    Serial.printf("M5_LCD::update() %d %d\n",n, stat );
    
    // 温度パネル スプライト準備
    TFT_eSprite PN = TFT_eSprite(&M5.Lcd);
    PN.setColorDepth(8);
    PN.createSprite( PAN_WIDTH, PAN_HEIGHT );
    makePanelBG( &PN, stat );
    PN.setTextColor( TFT_WHITE );
    PN.setTextFont( 1 );

    // フォントカラー設定
    //Serial.println("Select Font Color");
    //Serial.printf("PANEL_bgc %x\n",&PANEL_bgc);
    uint16_t fC = TFT_WHITE;
    uint16_t bC = PANEL_bgc[(int)stat];

    // 文字列変換
    //Serial.println("Data Traslation");
    if( dt->AVS > 99999 ) dt->AVS = 99999;   // AVS Limit
    char    str_templ[5];
    char    str_humid[5];
    char    str_templ_l[2];
    char    str_humid_l[2];
    char    str_press[5];
    char    str_als[6];
    sprintf( str_templ,   "%3d", (int)(dt->Templ) );
    sprintf( str_humid,   "%3d", (int)(dt->Humid) );
    sprintf( str_templ_l, "%1d", ftoa1(dt->Templ) );
    sprintf( str_humid_l, "%1d", ftoa1(dt->Humid) );
    sprintf( str_press,   "%4d", (int)(dt->Press) );
    sprintf( str_als,     "%4d", (int)(dt->AVS)   );

#ifdef SAST_DEBUG
    Serial.printf("templ : %s\r\n", str_templ );
    Serial.printf("templL: %s\r\n", str_templ_l );
    Serial.printf("humid : %s\r\n", str_humid );
    Serial.printf("humidL: %s\r\n", str_humid_l );
    Serial.printf("Press : %s\r\n", str_press );
    Serial.printf("AVS   : %s\r\n", str_als );
#endif

    // draw TEMPLATURE
    //Serial.println("Draw Templature");
    PN.drawChar(  2, 24 , str_templ[0], fC, bC, 3 );
    PN.drawChar( 17, 24 , str_templ[1], fC, bC, 4 );
    PN.drawChar( 43, 23 , str_templ[2], fC, bC, 4 );
    PN.drawChar( 62, 40 , '.',          fC, bC, 2 );
    PN.drawChar( 74, 40 , str_templ_l[0], fC, bC, 2 );
    PN.drawChar( 93, 40 , 'C',          fC, bC, 2 );
    PN.drawChar( 84, 25 , '.',          fC, bC, 2 );

    // draw HUMIDITY
    //Serial.println("Draw HUMIDITY");
    PN.drawChar( 33, 65 , str_humid[0], fC, bC, 3 );
    PN.drawChar( 51, 65 , str_humid[1], fC, bC, 3 );
    PN.drawChar( 71, 65 , str_humid[2], fC, bC, 3 );
    PN.drawChar( 93, 70 , '%', fC, bC, 2 );

    if ( dt->Type == SENS_t::TH1 ) {
        // draw Pressure
        //Serial.println("Draw Pressure");
        PN.drawChar( 40, 95 , str_press[0], fC, bC, 2 );
        PN.drawChar( 52, 95 , str_press[1], fC, bC, 2 );
        PN.drawChar( 64, 95 , str_press[2], fC, bC, 2 );
        PN.drawChar( 76, 95 , str_press[3], fC, bC, 2 );
        PN.drawChar( 87, 101 , 'h', fC, bC, 1 );
        PN.drawChar( 94, 101 , 'P', fC, bC, 1 );
        PN.drawChar(100, 101 , 'a', fC, bC, 1 );
    } else {
        // draw AVS
        //Serial.println("Draw AVS");
        PN.drawChar( 40, 95 , str_als[0], fC, bC, 2 );
        PN.drawChar( 52, 95 , str_als[1], fC, bC, 2 );
        PN.drawChar( 64, 95 , str_als[2], fC, bC, 2 );
        PN.drawChar( 76, 95 , str_als[3], fC, bC, 2 );
        PN.drawChar( 87, 95 , str_als[4], fC, bC, 2 );
        PN.drawChar( 94, 101 , 'L', fC, bC, 1 );
        PN.drawChar(100, 101 , 'X', fC, bC, 1 );
    }

    //LCDに転送
    //Serial.println("draw LCD");
    PN.pushSprite( PN_pos[n].x, PN_pos[n].y );

    // ============================== バッテリー＆電波強度
    int8_t BTLV=-1, RSSI=-1;
    if( dt->Type == SENS_t::Lazurite ) {
        BTLV = getBATT_lazurite( dt->batt );
        RSSI = getRSSI_lazurite( dt->RSSI );
        //Serial.printf("BTLV : %d\n",BTLV );
        //Serial.printf("RSSI : %d\n",RSSI );
    }

    // BATTアイコン（−1は未設定）
    if( BTLV != -1 ) {
        TFT_eSprite BATT = TFT_eSprite(&M5.Lcd);
        BATT.setColorDepth(8);
        BATT.createSprite( BATT_Width, BATT_Height );
        BATT.pushImage( 0, 0, BATT_Width, BATT_Height, BATT_img[BTLV]);
        BATT.pushSprite( PN_pos[n].x+86, PN_pos[n].y+3, BGC_TRANSP );
        BATT.deleteSprite();
    }

    //RSSIアイコン（-1は未設定）
    if( RSSI != -1 ) {
        TFT_eSprite ANTN = TFT_eSprite(&M5.Lcd);
        ANTN.setColorDepth(8);
        ANTN.createSprite( ANT_Width, ANT_Height );
        ANTN.pushImage( 0, 0, ANT_Width, ANT_Height, ANT_img[RSSI]);
        ANTN.pushSprite( PN_pos[n].x+64, PN_pos[n].y+3, BGC_TRANSP );
        ANTN.deleteSprite();
    }

    // メモリ解放
    PN.deleteSprite();
}

/**
 * @brief 画面再更新
 * @param all TRUEの時は全画面の再描画
 */
void M5_LCD::draw( bool all ) {
    drawPanel( all );
    drawStatusBar();
}

/**
 * @brief 画面全部の再描画
 */
void M5_LCD::reDraw() {
    clear();
    draw( true );
}

/**
 * @brief 画面のクリア（初期画面）
 */
void M5_LCD::clear() {

    TFT_eSprite PN = TFT_eSprite(&M5.Lcd);
    PN.setColorDepth(8);
    PN.createSprite( PAN_WIDTH, PAN_HEIGHT );
    makePanelBG( &PN, SSTAT_t::normal );

    TFT_eSprite STT = TFT_eSprite(&M5.Lcd);
    STT.setColorDepth(8);
    STT.createSprite( STT_WIDTH, STT_HEIGHT );
    STT.fillSprite(BGC_STT);
    STT.pushSprite( ST_pos.x, ST_pos.y );

    for( uint16_t i=0; i < MAX_SENS; i++ ) {
        PN.pushSprite( PN_pos[i].x, PN_pos[i].y );
    }

    PN.deleteSprite();
    STT.deleteSprite();

}

/**
 * @brief パネル情報の描画（更新分のみ）
 * @param all TRUE 全パネルの描画
 */
void M5_LCD::drawPanel( bool all ) {
    if( !all ) {
    // === 更新のみ描画
        for( int i=0; i < MAX_SENS; i++ ) {
            if( SENS->Sens[i].updated ) {
                update( i, SENS->Sens[i].status, &SENS->Sens[i].Data );
                SENS->Sens[i].updated = false;
            }
        }
    } else {
    // === 全描画
        for( int i=0; i < MAX_SENS; i++ ) {
            update( i, SENS->Sens[i].status, &SENS->Sens[i].Data );
        }
    }
}

/**
 * @brief ステータスバーの描画
 */
void M5_LCD::drawStatusBar() {
    //Serial.println("LCD::drawStatusBar()");
    // TODO mutex START

    // ステータスバー スプライト
    TFT_eSprite STM = TFT_eSprite(&M5.Lcd);
    STM.setColorDepth(8);
    STM.createSprite( STT_WIDTH, STT_HEIGHT );
    STM.fillSprite(BGC_STT);

    // 本体情報
    sData *dt = &SENS->EnvS.Data;
    char   buff[64];
    sprintf( buff, "%3.0fC %2.0f%% %4dhPa", dt->Templ, dt->Humid, dt->Press);    
    String envStr = buff;
    //Serial.println(envStr);

    // 温度文字列描画
    STM.setTextColor( TFT_DARKGREY, BGC_STAT );
    STM.setTextFont( 1 );
    STM.setTextSize( 1 );
    STM.setCursor( 0, 5 );
    STM.print( envStr );
    
    // 時刻情報更新
    if( RTC != NULL ) {
        RTC->calc();
        String timeStr = RTC->getTimeSTR();
        STM.setTextSize( 2 );
        STM.setCursor( LCD_WIDTH - STM.textWidth( timeStr.c_str() ), 2 );   // X=242
        STM.print( timeStr );

        //Serial.println( timeStr );
        // Serial.println( LCD_WIDTH - M5.Lcd.textWidth( timeStr.c_str() ) );
    }

    STM.pushSprite( ST_pos.x, ST_pos.y );
    STM.deleteSprite();
      
    // TODO mutex END
}

/**
 * @brief QRコード表示文字列設定（Ambient）
 * @param url URL文字列
 */
void M5_LCD::setURL( String url ) {
    URL = url;
}

/**
 * @brief LINE用QRコード表示文字列設定（LINE）
 * @param url 　URL文字列
 */
void M5_LCD::setLINE( String url ) {
    LINE = url;
}

/**
 * @brief グラフ表示用 QRコードを画面に表示
 * @note Cボタンを押すと、別のRQRを表示して切り替える
 */
void M5_LCD::showInfo(){
    uint8_t show = 0;
    const uint8_t MAX_QR = 2;
    while(1) {
        show = (show == MAX_QR) ? 0 : show;
        if( !showQR( show++ ) ) continue;
        BTN_t btn = wait_btnPress( 10 );
        if( btn == BTN_t::C ) continue;
        else break;
    }
    reDraw();
}

/**
 * @brief 
 * 
 * @param num 
 * @return true 
 * @return false 
 */
bool M5_LCD::showQR( uint8_t num ) {
    Serial.printf("showQR(%d)\n",num);
    switch( num ) {
        case 0:  return drawQR( URL, "Graph");
        case 1:  return drawQR( LINE, "LINE");
    }
}

/**
 * @brief QRコードとキャプションを30秒間表示する
 * @param url QRコードに表示するURL
 * @param caption 文字キャプション（未指定時は表示しない）
 */
bool M5_LCD::drawQR( String url, String caption ) {
    Serial.println("LCD::showQR()");
    Serial.println(url);
    Serial.println(caption);
    if( url.length() == 0) return false;       // URL未設定時は何もしない

    M5.Lcd.clear();
    M5.Lcd.qrcode( url, 50, 10, 220, 6 );

    if( caption.length() != 0 ) {
        M5.Lcd.setTextColor( TFT_DARKGREY, BGC_STAT );
        M5.Lcd.setTextFont( 1 );
        M5.Lcd.setTextSize( 2 );
        M5.Lcd.drawString( caption, 0, 0 );
    }
    return true;
}


/**
 * @brief 画面輝度の変更（呼び出し毎に5段階自動調整）
 * @param brt 0〜4を指定、-1の場合は1段階自動的に低くなる
 */
void M5_LCD::setBrightness( int8_t brt ){
    //Serial.printf("setBrightness(%d)\r\n",brt);
    if( brt == -1 ) {
        if ( light-- == 0 ) light = 4;
    } else {
        light = brt;
    }
    Serial.printf("Brightness %d\r\n",light);
    M5.Lcd.setBrightness(brightness[light]);
}

/**
 * @brif floatの小数点第1位をcharで返す
 * @param val:float
 */
uint8_t M5_LCD::ftoa1( float val ) {
    return (uint8_t)( (val - (int)val ) * 10 );
}

/**
 * @brief PANEL外観を描画する。
 * @param sp 描画スプライト
 * @param stat 温度ステータス
 */
void M5_LCD::makePanelBG( TFT_eSprite *sp, SSTAT_t stat ) {
    //Serial.printf("makePanelBG( %d ) \n", (int)stat);
    uint16_t bgc = PANEL_bgc[(int)stat];
    sp->fillSprite( bgc );
    sp->drawFastVLine( 0,           0, PAN_HEIGHT, BGC_LINE_L );
    sp->drawFastVLine( PAN_WIDTH-1, 0, PAN_HEIGHT, BGC_LINE_D ); 
    sp->drawFastHLine( 0,           0, PAN_WIDTH , BGC_LINE_L );
    sp->drawFastHLine( PAN_HEIGHT-1,0, PAN_WIDTH , BGC_LINE_D );
}


/**
 * @brief 最終更新からの経過時間の表示
 * 
 * @param n センサーNO
 * @param stat 状態
 * @param dt データ
 */
bool M5_LCD::view_state( uint16_t n, SSTAT_t stat, sData *dt ){
    Serial.printf("M5_LCD::view_state() %d %d\n",n, stat );

    // バッファ
    char buff[40];

    // 温度パネル スプライト準備
    TFT_eSprite PN = TFT_eSprite(&M5.Lcd);
    PN.setColorDepth(8);
    PN.createSprite( PAN_WIDTH, PAN_HEIGHT );
    makePanelBG( &PN, stat );
    PN.setTextColor( TFT_WHITE );
    PN.setTextFont( 0 );
    PN.setTextSize( 2 );

/*  処理入れたので不要になったはず
    // 更新からの経過時間計算
    if( dt->date != 0 ) {
      double sec = RTC->getTimeDiffer( dt->date );
      long min = sec /60;
      Serial.printf(" %d min / %f sec\n",min, sec);

      if( min > 5 ) makePanelBG( &PN, SSTAT_t::warn );
      sprintf( buff, "T:%2d",min);
      PN.setCursor( 5,15 );
      PN.print( buff );
    }
*/
    // バッテリー
    sprintf( buff, "B:%3.1f", dt->batt);
    PN.setCursor( 5,35 );
    PN.print( buff );

    // RSSI
    sprintf( buff, "R:%3d", dt->RSSI);
    PN.setCursor( 5,55 );
    PN.print( buff );


    // ============================== LCD描画
    //Serial.println("draw LCD");
    PN.pushSprite( PN_pos[n].x, PN_pos[n].y );

    // ============================== バッテリー＆電波強度
    int8_t BTLV=-1, RSSI=-1;
    if( dt->Type == SENS_t::Lazurite ) {
        BTLV = getBATT_lazurite( dt->batt );
        RSSI = getRSSI_lazurite( dt->RSSI );
        //Serial.printf("BTLV : %d\n",BTLV );
        //Serial.printf("RSSI : %d\n",RSSI );
    }

    // BATTアイコン（−1は未設定）
    if( BTLV != -1 ) {
        TFT_eSprite BATT = TFT_eSprite(&M5.Lcd);
        BATT.setColorDepth(8);
        BATT.createSprite( BATT_Width, BATT_Height );
        BATT.pushImage( 0, 0, BATT_Width, BATT_Height, BATT_img[BTLV]);
        BATT.pushSprite( PN_pos[n].x+86, PN_pos[n].y+3, BGC_TRANSP );
        BATT.deleteSprite();
    }

    //RSSIアイコン（-1は未設定）
    if( RSSI != -1 ) {
        TFT_eSprite ANTN = TFT_eSprite(&M5.Lcd);
        ANTN.setColorDepth(8);
        ANTN.createSprite( ANT_Width, ANT_Height );
        ANTN.pushImage( 0, 0, ANT_Width, ANT_Height, ANT_img[RSSI]);
        ANTN.pushSprite( PN_pos[n].x+64, PN_pos[n].y+3, BGC_TRANSP );
        ANTN.deleteSprite();
    }

    PN.deleteSprite();
}

/**
 * @brief センサーステータス状態の表示
 */
void M5_LCD::drawStat() {
    for( int i=0; i < MAX_SENS; i++ ) {
        view_state( i, SENS->Sens[i].status, &SENS->Sens[i].Data );
    }
    wait_btnPress( 15 );  // 15秒待機

    // 画面再表示
    reDraw();
}
