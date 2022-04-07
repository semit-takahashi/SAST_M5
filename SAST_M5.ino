/**
 *  @file SAST_M5.ino
 *  @author F.Takahashi (fumihito.takahashi@sem-it.com)
 *  @brief
 *  @version 0.1
 *  @date 2022-03-07
 *  @copyright Copyright (c) 2022 SEM-IT 
*/
#define _VERSION_ "2.3.0 20220407"
//#define DEBUG
//#define __SCREEN_SHOT
//#define TEST
//#define SAST_DEBUG

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

// タスクハンドル
TaskHandle_t taskHandle;
void loop2( void* arg );   // プロトタイプ

/*===================================================== Class ENV III SENS ===*/
class sens_EnvSensIII {
  private:
    SHT3X   sht30;
    QMP6988 qmp6988;
    bool    use = false;
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
  bool isExist(){return use;}
};

/* ================================================  Lazurite920J Class === */
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

  public:
    sData     data;

    // デコード
    sData * decode( String dt ) {
      //Serial.printf("L902::decode(%s)\r\n",dt.c_str());
      uint16_t cur = 0;
      uint16_t old = 0;
      String buff;
      dBuff = dt;

      // Sqeuens(HEX)
      cur = dt.indexOf(",");
      buff = ToDec(dt.substring(old, cur) );
      seq = buff.toInt();
      old = cur + 1;

      // 送信元Addr(HEX)-String
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      data.ID = buff;
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

      // センサータイプ
      data.Type = SENS_t::Lazurite;

      // デバッグダンプ
      sData::dump( &data );

      return &data;
    }
    // HEX to DEC
    static uint16_t ToDec(String str) {
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
};

/**
 * @brief Get the BATT Levle for lazurite
 * @param batt データ
 * @return int8_t 0~4アイコンindex
 */
int8_t getBATT_lazurite( float batt ) {
    if( batt >= 2.9 ) return 3;
    if( batt >= 2.7 ) return 2;
    if( batt >= 2.5 ) return 1;
    return 0;
}

/**
 * @brief Get the RSSI Levl for lazurite
 * @param rssi データ
 * @return int8_t 0~4アイコンindex
 */
int8_t getRSSI_lazurite( int16_t rssi ) {
    if( rssi >= 150 ) return 3;
    if( rssi >= 110 ) return 2;
    if( rssi >= 80  ) return 1;
    return 0;
}

/**
 * @brief ボタン押下orタイムアウトまでブロック
 * @param sec タイムアウト秒
 * @return BTN_t 終了イベント種類
 */
BTN_t wait_btnPress( uint16_t sec ) {
    for( int i=0; i < sec * 10 ; i++ ) {
        M5.update();
        if (M5.BtnA.wasPressed() ) return BTN_t::A;
        if (M5.BtnB.wasPressed() ) return BTN_t::B;
        if (M5.BtnC.wasPressed() ) return BTN_t::C;
        delay(100);
    }
    return BTN_t::OUT;
}
/* ======================================================================================= Globals */

// センサーリスト
SensList SENSORS;

// センサー：Lazurite
sens_L902J S_L920;

// センサー：環境センサ
sens_EnvSensIII S_ENV;

// 時計
netRTC RTC;

// 画面クラス
M5_LCD LCD;

// INI呼び出し
INF INI;




/* =========================================================================================setup */
void setup() {
  M5.begin();
  M5.Power.begin();
#ifdef __SCREEN_SHOT
  Serial.println("Use Screen shot!!");
  M5.ScreenShot.init( &M5.Lcd, M5STACK_SD );
  M5.ScreenShot.begin();
#endif

  // 初期画面設定
  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(BGC_STAT);
  M5.Lcd.setTextColor(TFT_WHITE, BGC_STAT );
  Serial.printf("SAST M5 Ver.%s\r\n", _VERSION_ );
  M5.Lcd.printf("SAST M5 Ver.%s\r\n", _VERSION_ );
  if (!M5.Power.canControl()) M5.Lcd.println(" ~~ Can't Power Control ~~");

// INI呼び出し
  M5.Lcd.print("\nLoad INI ... ");
  Serial.println("\nLoad INI ... ");
  INI.setSensorList( &SENSORS );   // Singletonにしても良いかも
  INI.load();
  M5.Lcd.println("done.");
  Serial.println("done.");
  Serial.println(INI.Name);

// DEBUG
  //SENSORS.dump();

  // 時刻設定(Wi-Fi接続)
  for( int i=0; i < 3; i++ ) {
    st_wifi ap;
    ap = INI.getWiFi( i );
    if( ap.ssid == NULL ) continue;
    RTC.setAP( ap.ssid.c_str(), ap.key.c_str() );
    if( RTC.setNTP() ) {
      break;
    }
  }
  if( !RTC.isSet() ) {
    Serial.printf("Wi-Fi is not Connect .");
    M5.Lcd.printf("Wi-Fi is not Connect .");
    while(1);
  }

  // シリアル通信機能2の設定（Lazurite通信部）
  // Serial2.begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert)
  Serial2.begin(115200, SERIAL_8N1, 16, 17 );
  //Serial2.setRxBufferSize(128);
  
  // ハンドシェーク
  Serial.println("setup Lazurite HOST");
  M5.Lcd.print("\nWakeup HOST ");
  Serial2.println("SAST");
  Serial2.flush();
  delay(300);

  for( int i=0; i<5; i++ ) {
    String buff = Serial2.readStringUntil('\n');
    //Serial.printf("buff:%s\r\n",buff.c_str());
    M5.Lcd.print('.');
    buff.trim();
    if( buff.startsWith("Ready") ) {
      M5.Lcd.println(buff);
      Serial.println(buff);
      break;
    }
    delay(800);
  }

  // 0.5秒待ち
  delay(500);
  
  // センサー類初期化 
  // 本体付属センサー初期化（ENV II）
  Serial.println("setup ENV II");
  S_ENV.setup();

  // 初期画面設定
  Serial.println("setup LCD Screen");
  LCD.init( &RTC, &SENSORS );
  LCD.draw(true);
  
  //Ambient初期化
  Serial.println(INI.amb_chID);
  Serial.println(INI.amb_wKey);
  
  //AMB.begin( INI.amb_chID, INI.amb_wKey.c_str(), &Client );
  //RTC.setAmbient( &AMB );
  RTC.setupAmbient( INI.amb_chID, INI.amb_wKey.c_str(), INI.amb_rKey.c_str());

  //LINE NOtify初期化
  RTC.setupNotify( INI.LINE_token.c_str() );


  // set QR Code
  Serial.printf("Graph:%s\n",INI.QRCode.c_str());
  LCD.setURL(INI.QRCode);

  // set LINE Group
  Serial.printf("LINE :%s\n",INI.LINE_GroupURL.c_str());
  LCD.setLINE(INI.LINE_GroupURL);

  // ネットワーク処理スレッド起動
  xTaskCreatePinnedToCore( loop2, "loop2", 8192, NULL, 1, &taskHandle, 1 );

  Serial.println("done setup()");
}


/* ===========================================================================================loop(Main1) */
void loop() {
  //Serial.println("Enter Loop()");

  static uint8_t env_count = 0;
  sData *dt = NULL;
  
  M5.update();

  // シリアル通信を受信したときの処理
  //Serial.println("Check Serial...");
  String buff;
  if (Serial2.available()) {
    buff = Serial2.readStringUntil('\n');
    buff.trim();
    if ( buff.length() != 0 ) {
      //Serial.print("Recv: "); Serial.println(buff);
      if( buff[0] != 'R' ) { 
        dt = S_L920.decode( buff );
        dt->date = RTC.getTimeRAW();    // 現在時刻を設定
        SENSORS.update( dt );
      }else{
        Serial.print("Recv -> Skip "); Serial.println(buff);
      }
    }
  }

  // IBS-TH!のデータ受信
  // スキャンしてデータ取得
  // MAC毎にデコード呼び出し

  // 本体センサーの読み出し(20回に1回)
  //Serial.printf("snv_count:%d\r\n",env_count);
  if( env_count++ % 20 == 0) {
    //Serial.println("loop::Check ENVIII");
    dt = S_ENV.getData();
    dt->date = RTC.getTimeRAW();    // 現在時刻を設定
    SENSORS.updateEnv( dt );
  }

  // TODO センサーの未反応チェック
  for( int i=0; i < SENSORS.Num; i++ ) {
    if( RTC.isElapsed( SENSORS.Sens[i].Data.date, 10 ) ) {
      // 経過している
      SENSORS.Sens[i].status == SSTAT_t::lost;
    }
  }

  // ボタンAが押された時の処理
  if (M5.BtnA.wasPressed()) {
    LCD.setBrightness();
  }

  // ボタンBが押された時の処理 詳細ステータス表示
  if (M5.BtnB.wasPressed()) {
    LCD.drawStat();

#ifdef __SCREEN_SHOT
      M5.ScreenShot.snap();
      Serial.println("Screen Shot!");
#endif
  }

  // ボタンCが押された時の処理
  if (M5.BtnC.wasPressed()) {
    LCD.showInfo();
  }

  // 画面更新
  //Serial.println("loop::Draw LCD");
  LCD.draw( );

  // Wait
  delay(500);
  //Serial.println("End Loop");
}

/* ===========================================================================================loop2 */
void loop2( void* arg ) {
  static time_t latest = RTC.getTimeRAW(); // 現時刻設定

  while(1) {
    //Serial.printf("latest: %d\n", latest );
    
    // Notifyの確認
    for( int i = 0; i < SENSORS.Num; i++ ) {
      if( SENSORS.Sens[i].status == SSTAT_t::caution ) {
        // 警告の場合は１分おき
        if( RTC.isElapsed( SENSORS.Sens[i].notify_time, 1 ) ) {
          Serial.printf("sendNotify(%d) CAUTION\n",i);
          String mess = makeNotifyMessage( INI.Name.c_str(), &SENSORS.Sens[i] );
          RTC.sendNotify(mess);
          SENSORS.Sens[i].notify_time = RTC.getTimeRAW();
        }
      }else if( SENSORS.Sens[i].status == SSTAT_t::warn ) {
        // 注意の場合は１０分おき
        if( RTC.isElapsed( SENSORS.Sens[i].notify_time, 10 ) ) {
          Serial.printf("sendNotify(%d) WARN\n",i);
          String mess = makeNotifyMessage( INI.Name.c_str(), &SENSORS.Sens[i] );
          RTC.sendNotify(mess);
          SENSORS.Sens[i].notify_time = RTC.getTimeRAW();
        }
      }else if( SENSORS.Sens[i].status == SSTAT_t::lost ) {
        // LOSTの場合は１０分おき
        if( RTC.isElapsed( SENSORS.Sens[i].notify_time, 10 ) ) {
          Serial.printf("sendNotify(%d) LOST",i);
          String mess = makeNotifyMessage( INI.Name.c_str(), &SENSORS.Sens[i] );
          RTC.sendNotify(mess);
          SENSORS.Sens[i].notify_time = RTC.getTimeRAW();
        }
      }
    }
  
    // 経過時間確認（1分間）
    if( !RTC.isElapsed( latest, 1 ) ) {
      delay(100);
      continue;
    }
    // センサーデータから Ambientデータ生成
    Serial.println("Send Ambient--");
    st_AMB dt[MAX_AMB];
    SENSORS.getAmbientData( dt );

    dt[0].dt = SENSORS.EnvS.Data.Templ;
    dt[0].use = true;
    dt[1].dt = SENSORS.EnvS.Data.Press;
    dt[1].use = true;

    for( int i=0; i < MAX_AMB; i++ ) {
      Serial.printf("%d:%5.2f(%s), ",i, dt[i].dt, dt[i].use ? "USE" : "NONE");
    }Serial.println("");

    RTC.sendAmbient( dt );

    // Google Spreadsheetにデータ送信（再送3回）
  
    // 無線LAN切断
  
    // Wait
    delay(100);
    latest = RTC.getTimeRAW();   // 時刻記録
  }
}

/**
 * @brief 通知用の文字列の作成
 * @param sns センサーデータ
 * @return String 生成した文字列
 */
String makeNotifyMessage( const char* Name, Sensor* s ) {
  if( s->status == SSTAT_t::normal ) return String("");
  //Serial.println("makeNotifyMessage()");

  String graph = "\n📊グラフ\n"+INI.QRCode;
  String mess;
  String head = String(Name)+String("\n");

  // 警告
  if( s->status == SSTAT_t::caution ) {
    mess = "🟥警告!【"+s->Name+"】が"+String(s->thr.caut_templ,1)+"℃を超えました(現在"+String(s->Data.Templ,1)+"℃)\n";

  // 注意
  }else if( s->status == SSTAT_t::warn ) {
    mess = "🟠注意!【"+s->Name+"】が"+String(s->thr.warn_templ,1)+"℃を超えました(現在"+String(s->Data.Templ,1)+"℃)\n";

  //センサーロスト
  }else if( s->status == SSTAT_t::lost ) {
    mess = "‼️センサー【"+s->Name+"(ID:"+s->ID+")】と接続できません\n";
  }
  return head + mess + graph;
}
