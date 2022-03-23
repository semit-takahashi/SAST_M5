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
const uint16_t *PANEL_img[] = { bg_pan_nor, bg_pan_warn, bg_pan_caut};
const uint16_t *ANT_img[] = {ANT_1, ANT_2, ANT_3, ANT_4};
const uint16_t *BATT_img[] = {BATT_0, BATT_1, BATT_2, BATT_3};

/**
 * @brief Construct a new M5_LCD::display object
 * @note スプライト情報の初期化
 */
M5_LCD::M5_LCD(){
    // パネルスプライト
    panel1.setColorDepth(8);
    panel1.createSprite( PAN_WIDTH, PAN_HEIGHT );
    panel1.pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, bg_pan_nor );
    panel1.setTextColor( TFT_WHITE );
    panel2.setColorDepth(8);
    panel2.createSprite( PAN_WIDTH, PAN_HEIGHT );
    panel2.pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, bg_pan_nor );
    panel2.setTextColor( TFT_WHITE );
    panel3.setColorDepth(8);
    panel3.createSprite( PAN_WIDTH, PAN_HEIGHT );
    panel3.pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, bg_pan_nor );
    panel3.setTextColor( TFT_WHITE );
    panel4.setColorDepth(8);
    panel4.createSprite( PAN_WIDTH, PAN_HEIGHT );
    panel4.pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, bg_pan_nor );
    panel4.setTextColor( TFT_WHITE );
    panel5.setColorDepth(8);
    panel5.createSprite( PAN_WIDTH, PAN_HEIGHT );
    panel5.pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, bg_pan_nor );
    panel5.setTextColor( TFT_WHITE );
    panel6.setColorDepth(8);
    panel6.createSprite( PAN_WIDTH, PAN_HEIGHT );
    panel6.pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, bg_pan_nor );
    panel6.setTextColor( TFT_WHITE );

    // ステータスバースプライト
    status.setColorDepth(8);
    status.createSprite( STT_WIDTH, STT_HEIGHT );
    status.pushImage( 0, 0, STT_WIDTH, STT_HEIGHT, bg_stt );
    status.setTextColor( TFT_WHITE );

    // 更新フラグ
    for( int i = 0; i < MAX_SENS; i++ ) {
        bUpdated[i] = false;
    }

    // 時刻情報
    RTC = NULL;

    // 輝度
    light = 4;
}


/**
 * @brief LCD初期化
 * 
 */
void M5_LCD::init(){
    // LCD初期化
    setBrightness( 2 );
    M5.Lcd.setTextFont(2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.fillScreen(TFT_NAVY);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_NAVY );
    M5.Lcd.setCursor(2, 2);

    // 画面情報表示
    draw( true );

    //輝度初期設定
    setBrightness( 3 );

}

/**
 * @brief netRTC設定（時刻取得用）
 * 
 * @param rtc 
 */
void M5_LCD::setRTC( netRTC *rtc ) {
    RTC = rtc;
}

/**
 * @brief センサー情報の更新してパネル情報を内部作成
 * @param n センサーNO（0〜5）
 * @param stat センサーステータス(NORMALWARN,CAUTION) 
 * @param dt センサデータ
 */
bool M5_LCD::update( uint8_t n, SSTAT_t stat, sData *dt ){
    Serial.println("updte()");
    sData::dump( dt );
    uint16_t fC = TFT_WHITE;
    uint16_t bC = BGC_PAN_NOR;

    // 文字列変換
    char    str_templ[5];
    char    str_humid[5];
    char    str_templ_l[2];
    char    str_humid_l[2];
    char    str_press[5];
    char    str_als[5];
    sprintf( str_templ,   "%3d", (int)dt->Templ );
    sprintf( str_humid,   "%3d", (int)dt->Humid );
    sprintf( str_templ_l, "%1d", ftoa1(dt->Templ) );
    sprintf( str_humid_l, "%1d", ftoa1(dt->Humid) );
    sprintf( str_press,   "%4d", (int)dt->Press );
    sprintf( str_als,     "%4d", (int)dt->AVS );

    // パネル選択
    TFT_eSprite *pan = PN[n];

    // BGのコピー
    pan->pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, PANEL_img[(int)stat] );

#ifdef DEBUG
    Serial.printf("templ : %s\r\n", str_templ );
    Serial.printf("templL: %s\r\n", str_templ_l );
    Serial.printf("humid : %s\r\n", str_humid );
    Serial.printf("humidL: %s\r\n", str_humid_l );
    Serial.printf("Press : %s\r\n", str_press );
    Serial.printf("AVS   : %s\r\n", str_als );
#endif

    // draw TEMPLATURE
    pan->drawChar(  2, 24 , str_templ[0], fC, bC, 3 );
    pan->drawChar( 17, 24 , str_templ[1], fC, bC, 4 );
    pan->drawChar( 43, 23 , str_templ[2], fC, bC, 4 );
    pan->drawChar( 62, 40 , '.',          fC, bC, 2 );
    pan->drawChar( 74, 40 , str_templ_l[0], fC, bC, 2 );
    pan->drawChar( 93, 40 , 'C',          fC, bC, 2 );
    pan->drawChar( 84, 25 , '.',          fC, bC, 2 );

    // draw HUMIDITY
    pan->drawChar( 33, 65 , str_humid[0], fC, bC, 3 );
    pan->drawChar( 51, 65 , str_humid[1], fC, bC, 3 );
    pan->drawChar( 71, 65 , str_humid[2], fC, bC, 3 );
    pan->drawChar( 93, 70 , '%', fC, bC, 2 );

    if ( dt->Type == SENS_t::TH1 ) {
        // draw Pressure
        pan->drawChar( 40, 96 , str_press[0], fC, bC, 2 );
        pan->drawChar( 52, 96 , str_press[1], fC, bC, 2 );
        pan->drawChar( 64, 96 , str_press[2], fC, bC, 2 );
        pan->drawChar( 76, 96 , str_press[3], fC, bC, 2 );
        pan->drawChar( 87, 101 , 'h', fC, bC, 1 );
        pan->drawChar( 94, 101 , 'P', fC, bC, 1 );
        pan->drawChar(100, 101 , 'a', fC, bC, 1 );
    } else {
        // draw AVS
        pan->drawChar( 40, 96 , str_als[0], fC, bC, 2 );
        pan->drawChar( 52, 96 , str_als[1], fC, bC, 2 );
        pan->drawChar( 64, 96 , str_als[2], fC, bC, 2 );
        pan->drawChar( 76, 96 , str_als[3], fC, bC, 2 );
        pan->drawChar( 94, 101 , 'L', fC, bC, 1 );
        pan->drawChar(100, 101 , 'X', fC, bC, 1 );
    }

    //BATT TODO
    pan->pushImage( 86, 3, BATT_Width, BATT_Height, BATT_img[0] );

    //ANT TODO
    pan->pushImage( 64, 3, ANT_Width, ANT_Height, ANT_img[0] );

}

/**
 * @brief M5本体の温度情報更新してステータスバー情報の作成
 * @param dt センサーデータ
 */
void M5_LCD::updateMyself( sData *dt ) {
    //Serial.println("updateMyself()");
    //sData::dump( dt );
    // 背景ロード
    status.pushImage( 0, 0, STT_WIDTH, STT_HEIGHT, bg_stt );

    // 温度データ文字列
    char   buff[64];
    sprintf( buff, "%3.0fC %3.0f%% %4dhPa", dt->Templ, dt->Humid, dt->Press);    
    String envStr = buff;
    //Serial.println(envStr);

    // 温度文字列描画
    status.setTextColor( TFT_DARKGREY, BGC_STAT );
    status.setTextFont( 1 );
    status.setTextSize( LCD_TXT_SIZE_ST );
    status.setPivot( 0, 0 );
    status.printToSprite( envStr );
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
 * 
 */
void M5_LCD::reDraw() {
    clear();
    draw( true );
}

/**
 * @brief 画面のクリア（消去）
 */
void M5_LCD::clear() {
    M5.Lcd.clear();
    M5.Lcd.fillScreen(BGC_STAT);
}

/**
 * @brief パネル情報の描画（更新分のみ）
 * @param all TRUE 全パネルの描画
 */
void M5_LCD::drawPanel( bool all ) {
    Serial.printf("LCD::drawPanel(%d)", all);
    if( !all ) {
    // === 更新のみ描画
        for( int i=0; i < MAX_SENS; i++ ) {
            if( bUpdated[i] ) {
                // TODO mutex START
                // 指定したNOのパネル情報を転送
                PN[i]->pushSprite( PN_pos[i].x, PN_pos[i].y );
                // TODO mutex END
                bUpdated[i] = false;
            }
        }
    } else {
    // === 全描画
        for( int i=0; i < MAX_SENS; i++ ) {
            // TODO mutex START
            PN[i]->pushSprite( PN_pos[i].x, PN_pos[i].y );
            // TODO mutex END
        }
    }
}

/**
 * @brief ステータスバーの描画
 */
void M5_LCD::drawStatusBar() {
    Serial.println("LCD::drawStatusBar()");
    // TODO mutex START
    // 時刻情報更新
    if( RTC != NULL ) {
        RTC->calc();
        String timeStr = RTC->getTimeSTR();
        status.setTextColor( TFT_DARKGREY, BGC_STAT );
        status.setTextFont( 1 );
        status.setTextSize( LCD_TXT_SIZE_ST );
        status.setPivot( LCD_WIDTH - M5.Lcd.textWidth( timeStr.c_str() ), 2 );
        status.printToSprite( timeStr );
    }
    status.pushSprite( ST_pos.x, ST_pos.y );
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
 * @brief グラフ表示用 QRコードを画面に表示
 */
void M5_LCD::showURL(){
    showQR( URL, "Graph" ); 
}

/**
 * @brief LINE用QRコード表示文字列設定（LINE）
 * @param url 　URL文字列
 */
void M5_LCD::setLINE( String url ) {
    LINE = url;
}

/**
 * @brief 通医LINEグループ QRコードを画面に表示
 */
void M5_LCD::showLINE(){
    showQR( LINE, "LINE" ); 
}


/**
 * @brief QRコードとキャプションを30秒間表示する
 * @param url QRコードに表示するURL
 * @param caption 文字キャプション（未指定時は表示しない）
 */
void M5_LCD::showQR( String url, String caption ) {
    Serial.println("LCD::showQR()");
    Serial.println(url);
    Serial.println(caption);
    if( url.length() == 0) return;       // URL未設定時は何もしない

    M5.Lcd.clear();
    M5.Lcd.qrcode( URL, 50, 10, 220, 6 );
    // 

    if( caption.length() != 0 ) {
        M5.Lcd.setTextColor( TFT_DARKGREY, BGC_STAT );
        M5.Lcd.setTextFont( 1 );
        M5.Lcd.setTextSize( 2 );
        M5.Lcd.drawString( caption, 0, 0 );
    }
    // TODO ボタン押下遷移も考慮
    delay( 15*1000 ); // 30秒表示

    // 画面再表示
    reDraw();
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


#if 0
void drawGraph( TFT_eSprite *pan, float dt , uint16_t clr, uint16_t minute, GRAPH_t mode)
{
    switch ( mode ) {
    // 温度グラフ
    case GRAPH_t::TEMPL:
        // 時刻から座標計算（温度の場合）
        uint16_t x = 21 + (minute * 0.194444);
        uint16_t y = 87 - (uint16_t)(dt / 0.555555);
        // 点の描画
        Serial.printf(" drawPixcel ( %d, %d )\r\n", x, y );
        pan->drawPixel( x, y, clr );
        break;
    }
}
#endif __GRAPH__
