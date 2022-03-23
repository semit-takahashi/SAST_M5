/**
 *  @file SAST_M5.ino
 *  @author F.Takahashi (fumihito.takahashi@sem-it.com)
 *  @brief
 *  @version 0.1
 *  @date 2022-03-07
 *  @copyright Copyright (c) 2022 SEM-IT 
*/
#define _VERSION_ "2.0.0 20220307"
//#define DEBUG
//#define TEST

#include <M5Stack.h>
#include "SAST_M5.h"

#include <Wire.h>
#include "UNIT_ENV.h"   //環境センサーIII用

#include "INF.h"
#include "netRTC.h"
#include "display.h"
#include "sensor.h"


// MUTEX（画面描画時利用）
portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;


/*===================================================== Class ENV III SENS ===*/
class sens_EnvSensIII {
  private:
    SHT3X   sht30;
    QMP6988 qmp6988;
    bool    use = false;

  public:
    sData   dt;

  public:
    bool setup() {
      if( !qmp6988.init() ) {
        Serial.println("Could not find a valid BQMP6988 sensor, check wiring!");
        use = false;
        return use;
      }

      Serial.println("Ready! ENV III Sensor!");
      use = true;
      return use;
    }

  sData *getData() {
    if ( !use ) NULL;

    dt.Press = qmp6988.calcPressure() / 100.0;
    if (sht30.get() == 0) {
      dt.Templ = sht30.cTemp;
      dt.Humid = sht30.humidity;
    }
    return &dt;
  }

  bool isExist() {
    return use;
  }
};

/* ==========================  Lazurite920J Class === */
class sens_L902J {
  public:
    uint16_t  seq = 0;        //シーケンス番号（発信元のシーケンス）
    uint16_t  src_addr = 0;   // 送信元のアドレス（下位 2バイト）
    uint16_t  rssi = 0;       // RSSI（255〜0 40未満は怪しい）
    float     templ = 0.0;    // 温度（小数点1桁）
    float     humid = 0.0;    // 湿度（小数点1桁）
    uint16_t  pres = 0;       // 気圧（hPa）
    float     als = 0;        // 照度（0〜XXXX lx） ALS：Ambient Light Sensor :
    uint16_t  ps = 0;         // 近接（）PS：Proximity Switch
    float     batt = 0.0;     // バッテリー電圧V
    String    dBuff;          // 受信データのバッファ
    struct tm atime;          // 受信日時（NTPから取得した時刻）

  public:
    sData     data;

    // デコード
    sData * decode( String dt ) {
      Serial.printf("L902::decode(%s)\r\n",dt.c_str());
      uint16_t cur = 0;
      uint16_t old = 0;
      String buff;
      dBuff = dt;

      // Sqeuens(HEX)
      cur = dt.indexOf(",");
      buff = ToDec(dt.substring(old, cur) );
      seq = buff.toInt();
      old = cur + 1;

      // 送信元Addr(HEX)
      cur = dt.indexOf(",", old );
      buff = ToDec(dt.substring(old, cur) );
      src_addr = buff.toInt();
      old = cur + 1;

      // RSSI
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      rssi = buff.toInt();
      data.RSSI = rssi;
      old = cur + 1;

      // 温度（℃）
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      templ = buff.toFloat();
      data.Templ = templ;
      old = cur + 1;

      // 湿度（％）
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      humid = buff.toFloat();
      data.Humid = humid;
      old = cur + 1;

      // 気圧（hPa）
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      pres = buff.toInt();
      data.Press = pres;
      old = cur + 1;

      // Ambient Light（LUX）
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      als = buff.toFloat();
      data.AVS = als;
      old = cur + 1;

      // Proximity（近接距離）
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      ps = buff.toInt();
      old = cur + 1;

      // バッテリー（V）
      buff = dt.substring(old, dt.length());
      batt = buff.toFloat();
      data.batt = batt;

      sData::dump( &data );

      return &data;
    }

    // 格納データのダンプ
    void dump() {

    }

    // HEX to DEC
    static uint16_t ToDec(String str)
    {
      short i = 0;        /* 配列の添字として使用 */
      short n = 0;
      unsigned long x = 0;
      char c;

      while (str[i] != '\0') {        /* 文字列の末尾でなければ */
        if ('0' <= str[i] && str[i] <= '9')  n = str[i] - '0';
        else if ('a' <= (c = tolower(str[i])) && c <= 'f')  n = c - 'a' + 10;
        else return -1;
        i++;        /* 次の文字を指す */
        x = x * 16 + n;   /* 桁上がり */
      }
      return (x);
    }

    void time2str( char* buff )
    {
      sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d.000",
              1900 + atime.tm_year, atime.tm_mon, atime.tm_mday,
              atime.tm_hour, atime.tm_min, atime.tm_sec );
    }
};



/* ======================================================================================= Globals */

// センサーリスト
SensList SENSORS;

// センサー：Lazurite
sens_L902J S_L920;

// センサー：環境センサ
sens_EnvSensIII S_ENV;

// 時計
netRTC rtc;

// 画面クラス
M5_LCD LCD;

// INI呼び出し
INF ini;

#if 0
// タイマー割込
Ticker  int_SBar;
bool sClock = false;    // 秒点滅用フラグ
Ticker  int_EnvSens;
#endif


#if 0
/* ==================================================================================== STATUS BAR */
void drawStatusBar() {
  uint8_t   txtSize = 2;
  uint16_t  txtFont = 1;
  uint16_t  x;
  uint16_t  y;
  char   buff[64];

  // 更新は別途タイマー割込で（時間がかかりすぎるため）
  //env.update();

  // 日時
  String timestr = String( rtc.getTimeSTR() );
  if ( sClock = !sClock ) timestr.setCharAt( 8 , ' ' ); // 秒の描画（書いたり消したり交互）

  //バッテリー
  String battstr = String( M5.Power.getBatteryLevel() );
  battstr += String( "%" );

  // 本体側環境
  String envstr;
  if ( env.isExist() ) {
#ifdef DEBUG
    sprintf( buff, "%3.0fC %3.0f%% %4dhPa %d", env.templ, env.humid, env.pres, esp_get_free_heap_size());
#else
    sprintf( buff, "%3.0fC %3.0f%% %4dhPa", env.templ, env.humid, env.pres);
#endif
    envstr = String( buff );
  }

  // クリティカルセクション（LCD描画）
  portENTER_CRITICAL_ISR(&mutex);

  M5.Lcd.setTextColor( TFT_DARKGREY, BGC_STAT );
  M5.Lcd.setTextFont( 1 );
  M5.Lcd.setTextSize( txtSize );
  x = LCD_WIDTH - M5.Lcd.textWidth( timestr.c_str() );
  y = LCD_HEIGHT - txtSize * LCD_TXT_SIZE;
  M5.Lcd.drawString( timestr, x, y );

  txtSize = 1;
  M5.Lcd.setTextSize( txtSize );
  y = LCD_HEIGHT - txtSize * LCD_TXT_SIZE - 4 ;
  M5.Lcd.drawString( battstr, 1, y );

  if ( envstr.length() != 0 ) {
    M5.Lcd.drawString( envstr, 30, y );
  }
  // クリティカルセクション終了
  portEXIT_CRITICAL_ISR(&mutex);

  //Serial.printf("drawStatus() %d ms\r\n", millis()-millis_start);

}
#endif



/* =========================================================================================setup */
void setup() {
  M5.begin();
  M5.Power.begin();

  // シリアル通信機能の設定
  //Serial.begin(115200);

  // INI呼び出し
  ini.setSensorList( &SENSORS );   // Singletonにしても良いかも
  ini.load();

  // シリアル通信機能2の設定
  // Serial2.begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert)
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  // 初期画面設定
  M5.Lcd.setTextSize(2);
  Serial.printf("SAST M5 Ver.%s\r\n", _VERSION_ );
  M5.Lcd.printf("SAST M5 Ver.%s\r\n", _VERSION_ );
  if (!M5.Power.canControl()) M5.Lcd.println(" ~~ Can't Power Control ~~");
  delay(500);

  // 時刻設定(Wi-Fi接続)
  for( int i=0; i < 3; i++ ) {
    st_wifi ap;
    ap = ini.getWiFi( i );
    if( ap.ssid == NULL ) continue;
    rtc.setAP( ap.ssid.c_str(), ap.key.c_str() );
    if( rtc.setNTP() ) {
      break;
    }
  }
  if( !rtc.isSet() ) {
    Serial.printf("Wi-Fi is not Connect .");
    M5.Lcd.printf("Wi-Fi is not Connect .");
    while(1);
  }

  // センサー類初期化 
  // 本体付属センサー初期化（ENV II）
  Serial.println("setup ENV II");
  S_ENV.setup();

  // 初期画面設定
  Serial.println("setup LCD Screen");
  LCD.clear();
  LCD.draw( true );   // 全描画

  
  //M5.Lcd.drawBmpFile( SD, "/SAST_BACK.bmp", 0,0 );

  // Debug
  LCD.setURL("https://www.sem-it.com");


  Serial.println("done setup()");




}


/* ===========================================================================================loop */
void loop() {
  Serial.println("Enter Loop()");
  static uint8_t env_count = 0;
  sData *dt = NULL;
  M5.update();

  // シリアル通信を受信したときの処理
  Serial.println("Check Serial...");
  String buff;
  if (Serial2.available()) {
    buff = Serial2.readStringUntil('\n');
    buff.trim();
    if ( buff.length() != 0 ) {
      //Serial.println("Recv Data");
      dt = S_L920.decode( buff );
      SENSORS.update( dt );
    }
  }

  // IBS-TH!のデータ受信
  // スキャンしてデータ取得
  // MAC毎にデコード呼び出し

  // 本体センサーの読み出し(20回に1回)
  //Serial.printf("snv_count:%d\r\n",env_count);
  if( env_count++ % 20 == 0) {
    Serial.println("Check ENVIII");
    dt = S_ENV.getData();
    LCD.updateMyself( dt );
  }


  // ボタンAが押された時の処理
  Serial.println("Check Button..");
  if (M5.BtnA.wasPressed()) {
    LCD.setBrightness();
  }

  // ボタンBが押された時の処理：Clear
  if (M5.BtnB.wasPressed()) {
  }

  // ボタンCが押された時の処理：Clear
  if (M5.BtnC.wasPressed()) {
    LCD.showURL();
  }

  // 画面更新]
  Serial.println("Draw LCD");
  LCD.draw();

  // 
  delay(500);
  Serial.println("End Loop");
}

/* ======================================================== Sensor Decorder *///
