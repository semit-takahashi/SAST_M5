/* F.Takahashi SAST V2 
 * File : SAST_M5.ino
*/
#include <M5Stack.h>
#include <WiFi.h>
#include <IniFile.h>
#include "time.h"

#include <Wire.h>
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include "UNIT_ENV.h"

const uint16_t LCD_WIDTH      = 320;
const uint16_t LCD_HEIGHT     = 240;
const uint16_t LCD_TXT_SIZE = 7;
const uint16_t CHART_WIDTH    = 250;
const uint16_t CHART_HEIGHT   = 100;

//const char *WF_SSID = "aterm-0eb815-g";
const char *WF_KEY  = "23d4a4031f7ab";

/*===================================================== Class Init Info ===*/
struct thrd {
  bool use = false;
  float min;
  float max;
};

struct SENS_TH1 {
  char  MAC[19];
  char  name[60];
  char  amb_templ[3];
  char  amb_humid[3];
  struct thrd th_templ;
  struct thrd th_humid;
};

struct SENS_920J {
  uint16_t  ADDR;
  char    name[60];
  char  amb_templ[3];
  char  amb_humid[3];
  char  amb_als[3];
  char  amb_press[3];
  struct thrd th_templ;
  struct thrd th_humid;
  struct thrd th_als;
};

class INF {
  private:
    const char *iniFile = "/setup.ini";

  public:
    String wifi_ssid;
    String wifi_key;

    // ambient
    String  amb_chID;
    String  amb_wKey;
    String  amb_rKey;

    // LINE
    String  LINE_token;
    String  LINE_URL;

    // Google Spredseet
    char  GS_token[64];

    // INKBIRD
    struct SENS_TH1   snse_th1[3];

    // L902J
    struct SENS_920J  sens_920j[3];

    INF() {}

    bool load()
    {
      const size_t  buff_len = 809;
      char          buff[buff_len];

      Serial.println("Load INI file.");
      if (!SD.begin()) {
        Serial.println(" Please Insert SD-CARD and RESET! ");
        M5.Lcd.println(" Please Insert SD-CARD and RESET! ");
        while (1) ;
      }

      IniFile in(iniFile);

      // File Open
      if (!in.open()) {
        Serial.printf("ERROR Ini file [%s] does not exist. \r\n STOP!", iniFile);
        while (1);
      }
      Serial.printf("INI file [ %s ] exists\r\n",iniFile);

      // Check File is Valid.
      if (!in.validate(buff, buff_len)) {
        Serial.printf("ini file %s not valid\r\n", in.getFilename());
        printErrorMessage(in.getError());
        // Cannot do anything else
        while (1);
      }

      // Wi-Fi
      if ( in.getValue( "Wi-Fi", "SSID", buff, buff_len ) ) {
        Serial.print("Wi-Fi SSID: ");
        Serial.println( buff );
        wifi_ssid = buff;
      } else {
        Serial.printf("Not found SSID : ");
        printErrorMessage(in.getError());
      }

      if ( in.getValue( "Wi-Fi", "KEY", buff, buff_len ) ) {
        Serial.print("Wi-Fi KEY : ");
        Serial.println( buff );
        wifi_key = buff;
      } else {
        Serial.printf("Not Found KEY : ");
        printErrorMessage(in.getError());
      }

    }

    void printErrorMessage(uint8_t e, bool eol = true) {
      switch (e) {
        case IniFile::errorNoError:
          Serial.print("no error");
          break;
        case IniFile::errorFileNotFound:
          Serial.print("file not found");
          break;
        case IniFile::errorFileNotOpen:
          Serial.print("file not open");
          break;
        case IniFile::errorBufferTooSmall:
          Serial.print("buffer too small");
          break;
        case IniFile::errorSeekError:
          Serial.print("seek error");
          break;
        case IniFile::errorSectionNotFound:
          Serial.print("section not found");
          break;
        case IniFile::errorKeyNotFound:
          Serial.print("key not found");
          break;
        case IniFile::errorEndOfFile:
          Serial.print("end of file");
          break;
        case IniFile::errorUnknownError:
          Serial.print("unknown error");
          break;
        default:
          Serial.print("unknown error value");
          break;
      }
      if (eol)
        Serial.println();
    }
};



/*===================================================== Class ENV II SENS ===*/
class envSens {
  private:
    SHT3X   sht30;
    Adafruit_BMP280 bme;
    bool    use = false;

  public:
    float     templ = 0.0f;
    float     humid = 0.0f;
    uint16_t  pres  = 0.0f;

  public:
    bool setup() {
      if ( !bme.begin(0x76) ) {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        use = false;
        return false;
      }
      Serial.println("Ready! ENV Sensor!");
      use = true;
      return true;
    }

    void update() {
      if ( !use ) return;
      pres = bme.readPressure() / 100.0;
      if (sht30.get() == 0) {
        templ = sht30.cTemp;
        humid = sht30.humidity;
      }
    }

    void view() {
      if ( !use ) return;
      char    dat1[20];
      char    dat2[20];
      char    dat3[20];

      update();
      sprintf(dat1, "%5.1f C",  templ);
      sprintf(dat2, "%5.1f %%", humid);
      sprintf(dat3, "%4d hPa",  pres);

      uint16_t xx = 200;
      uint16_t yy = 25;
      //M5.Lcd.setTextFont( 2 );
      M5.Lcd.setTextSize( 3 );
      M5.Lcd.drawString( dat1, xx, yy    );
      M5.Lcd.drawString( dat2, xx, yy + 30 );
      M5.Lcd.drawString( dat3, xx, yy + 60 );
    }
};


/*===================================================== Class NTP Timeset ===*/
class netRTC {
  private:
    // UpdateTiming
    const unsigned long rst_hour = 600 * 1000;    // 10minute
    //const unsigned long rst_hour = 2.16e7;    // 6hour
    bool        IsSet = false;
    const char*       ssid;
    const char*       key;
    const char* ntpServer =  "ntp.jst.mfeed.ad.jp";
    const long  gmtOffset_sec = 3600 * 9;   // JST +9:00
    const int   daylightOffset_sec = 0;
    char        str_stime[16];      // 短い時間 MM/DD hh:mm
    char        str_ltime[24];      // 長い時間 YYYY-MM-DD hh:mm:ss.ttt
    unsigned int rst_count = 0;     // reset counter

  public:
    void setAP( const char *wifi_ssid, const char *wifi_key ) {
      ssid = wifi_ssid;
      key = wifi_key;
      IsSet = true;
      Serial.println( ssid );
      Serial.println( key );
    }

    bool setNTP() {
      if ( !IsSet ) return false;
      uint8_t cnt = 20;
      IPAddress ip;

      // connect to WiFi
      M5.Lcd.printf("connecting to\r\n %s ", ssid);
      Serial.printf("Connecting to %s ", ssid);
      WiFi.begin(ssid, key);
      while (WiFi.status() != WL_CONNECTED) {
        if ( cnt-- == 0 ) {
          WiFi.disconnect(true);
          WiFi.mode(WIFI_OFF);
          M5.Lcd.printf(" Connect ERROR! (%d)\r\n", WiFi.status());
          Serial.printf(" Connect ERROR! (%d)\r\n", WiFi.status());
          // BEEP!!
          delay( 10000 );
          return false;
        }
        delay(500);
        M5.Lcd.printf(".");
        Serial.print(".");
      }
      ip = WiFi.localIP();
      M5.Lcd.print(" CONNECTED -> ");
      M5.Lcd.println(ip);
      Serial.print(" CONNECTED -> ");
      Serial.println(ip);
      // Set ntp time to local
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      struct tm timeInfo;
      if (getLocalTime(&timeInfo)) {
        M5.Lcd.printf("NTP %s\r\n\r\n", ntpServer);
        delay(500);
      }
      //disconnect WiFi
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
    }

    void calc() {
      struct tm tf;
      if (!getLocalTime(&tf)) {
        M5.Lcd.println("Failed to obtain time");
        Serial.println("ERROR: Faild to obtain time");
        return;
      }
      sprintf(str_stime, "%2d/%2d %2d:%02d", tf.tm_mon, tf.tm_mday, tf.tm_hour, tf.tm_min );
      sprintf(str_ltime, "%04d-%02d-%02d %02d:%02d:%02d.000", 1900 + tf.tm_year, tf.tm_mon, tf.tm_mday, tf.tm_hour, tf.tm_min, tf.tm_sec );
    }

    char* getTimeSTR()
    {
      calc();
      return str_stime;
    }

    // NTP時間の再設定を実施
    void reflesh() {
      // 更新時間を経過した？
      unsigned int hour = millis() / rst_hour;
      Serial.printf("ref: %d(%fd)\r\n", hour, rst_count );
      // タイマーが戻った？（50日経過した？）
      if ( hour == 0 && rst_count != 0 ) rst_count = 0;
      if ( hour > rst_count ) {
        uint8_t cnt = 20;
        Serial.print("Reflesh TIME: set NTP ");
        rst_count++;
        WiFi.begin(ssid, key);
        while (WiFi.status() != WL_CONNECTED) {
          if ( cnt-- == 0 ) {
            Serial.printf("\r\n ERROR Not Connect(%d) skip!!\ｒ\n", WiFi.status());
            return;
          }
          delay(500);
        }
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        Serial.println(" .. done.");
      }
    }

    void viewTime()
    {
      calc();
      uint8_t txtSize = 2;
      M5.Lcd.setTextColor( TFT_DARKGREY, TFT_BLACK);
      M5.Lcd.setTextSize( txtSize );
      uint16_t x = LCD_WIDTH - M5.Lcd.textWidth(str_stime);
      uint16_t y = LCD_HEIGHT - txtSize * LCD_TXT_SIZE;
      M5.Lcd.drawString( str_stime, x, y );
    }
};


/* ==========================  Lazurite920J Class === */
class L902J {
  public:
    uint16_t  seq = 0;        //シーケンス番号（発信元のシーケンス）
    uint16_t  src_addr = 0;   // 送信元のアドレス（下位 2バイト）
    uint16_t  rssi = 0;       // RSSI（255〜0 40未満は怪しい）
    float     templ = 0.0;    // 温度（小数点1桁）
    float     humid = 0.0;    // 湿度（小数点1桁）
    uint16_t  pres = 0;       // 気圧（hPa）
    float     lux = 0;        // 照度（0〜XXXX lx） ALS：Ambient Light Sensor :
    uint16_t  ps = 0;         // 近接（）PS：Proximity Switch
    float     batt = 0.0;     // バッテリー電圧V
    String    dBuff;          // 受信データのバッファ
    struct tm atime;          // 受信日時（NTPから取得した時刻）

  public:
    // コンストラクタ
    L902J(void) {
    }

    // コピーコンストラクタ
    L902J( String data ) {
      decode( data );
    }

    // デコード
    void decode( String dt ) {
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
      old = cur + 1;

      // 温度（℃）
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      templ = buff.toFloat();
      old = cur + 1;

      // 湿度（％）
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      humid = buff.toFloat();
      old = cur + 1;

      // 気圧（hPa）
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      pres = buff.toInt();
      old = cur + 1;

      // Ambient Light（LUX）
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      lux = buff.toFloat();
      old = cur + 1;

      // Proximity（近接距離）
      cur = dt.indexOf(",", old );
      buff = dt.substring(old, cur);
      ps = buff.toInt();
      old = cur + 1;

      // バッテリー（V）
      buff = dt.substring(old, dt.length());
      batt = buff.toFloat();

      // 時刻の記録
      if (!getLocalTime(&atime)) {
        Serial.println("[L920J] ERROR: Faild to obtain time");
        return;
      }

    }
    void view()
    {
      char sens[30];
      char dat1[10];
      char dat2[10];
      char dat3[10];
      sprintf( sens, "%4x(%2x) %3d [%3.1fV]", src_addr, seq, rssi, batt);
      sprintf( dat1, "%5.1f C", templ);
      sprintf( dat2, "%5.1f %%", humid);
      sprintf( dat3, "%4.0f Lx", lux);
      M5.Lcd.setTextColor( DARKGREY, BLACK );
      M5.Lcd.setCursor( 2, 2 );
      M5.Lcd.setTextSize( 2 );
      M5.Lcd.println(sens);

      uint8_t yy = 25;
      M5.Lcd.setTextSize( 3 );
      M5.Lcd.drawString( dat1, 2, yy    );
      M5.Lcd.drawString( dat2, 2, yy + 30 );
      M5.Lcd.drawString( dat3, 2, yy + 60 );
    }

    // 格納データのダンプ
    void dump()
    {
      TFT_eSprite d = TFT_eSprite(&M5.Lcd);
      d.setColorDepth(8);
      d.createSprite( LCD_WIDTH - 10 , LCD_HEIGHT - 10 );
      d.fillSprite( TFT_NAVY );
      d.setTextColor( TFT_WHITE );
      d.setCursor( 0, 0 );
      d.setTextFont( 1 );
      d.setTextSize( 2 );

      char  atime_str[24];
      time2str(atime_str);
      //d.printf("buff:");
      //d.println(dBuff);
      d.printf("%s\r\n", atime_str);
      d.printf("seq   %3d\r\n", seq);
      d.printf("addr  %x\r\n", src_addr);
      d.printf("rssi  %d\r\n", rssi);
      d.printf("templ %4.1f\r\n", templ);
      d.printf("humid %4.1f\r\n", humid);
      d.printf("press %4d\r\n", pres);
      d.printf("lux   %6.1f\r\n", lux);
      d.printf("ps    %3d\r\n", ps);
      d.printf("BATT  %4.1f\r\n", batt);

      d.pushSprite( 5, 5 );
      delay( 5000 );
      d.fillSprite( TFT_BLACK );
      d.pushSprite( 5, 5 );
      d.deleteSprite();

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
      sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d.000", 1900 + atime.tm_year, atime.tm_mon, atime.tm_mday, atime.tm_hour, atime.tm_min, atime.tm_sec );
    }
};

/* ==================================== 描画クラス === */
class DrawGraph {
  private:
    uint16_t x0, y0, x1, y1, wd, ht = 0;
    uint16_t bg_color = TFT_NAVY;       // 背景色
    uint16_t bd_color = TFT_WHITE;    // 境界色

  public:
    DrawGraph() {}
    DrawGraph( uint16_t x_0, uint16_t y_0, uint16_t width, uint16_t height ) {
      setView( x_0, y_0, width, height );
    }

    void setView( uint16_t x_0, uint16_t y_0, uint16_t width, uint16_t height ) {
      x0 = x_0;
      y0 = y_0;
      x1 = x_0 + width;
      y1 = y_0 + height;
      wd = width;
      ht = height;
      Serial.printf("setView: %d, %d, %d, %d\r\n", x0, y0, x1, y1);
    }

    void clear()     {
      M5.Lcd.setTextSize(2);
      M5.Lcd.fillScreen(TFT_BLACK);
      M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.setCursor(0, 0);
      // Rect -> ( x, y, width, height )
      M5.Lcd.fillRect( x0, y0, wd, ht , bg_color );
      M5.Lcd.drawRect( x0, y0, wd, ht , bd_color );
    }

    void plot( uint16_t count, float dt, float min, float max, uint16_t color ) {
      uint16_t x, y;

      x = count;
      y = y1 - ( ht / ( max - min ) * dt );
      Serial.printf("plot( %d, %f ) -> ( %d, %d )\r\n", count, dt, x , y );
      M5.Lcd.drawPixel( x , y , color );
    }

};

/* ======================================================================================= Globals */
// 受信データ最新
L902J n_data;

// 時計
netRTC rtc;

// グラフ
//rawGraph grp;

//グラフスプライト
TFT_eSprite chart = TFT_eSprite(&M5.Lcd);

//環境センサ
envSens env;

// INI呼び出し
INF ini;

// 描画回数
uint32_t count = 0;

/* =========================================================================================setup */
void setup() {
  M5.begin();
  M5.Power.begin();

#if 0
  // SD card CHECK
  if (!SD.begin()) {
    M5.Lcd.println(" Please Insert SD-CARD and RESET! ");
    while (1) ;
  }

  // 漢字フォント読み込み
  M5.Lcd.println("Load font ... ");
  String FontName = "/vlw/genshin-regular-20pt";
  M5.Lcd.loadFont( FontName, SD );

  // 画面初期設定
  M5.Lcd.fillScreen(TFT_WHITE);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.printf("SAST日本語版〜開始！");
#endif

  // シリアル通信機能の設定
  //Serial.begin(115200);

  // INI呼び出し
  ini.load();

  // シリアル通信機能2の設定
  // Serial2.begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert)
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  Serial.println("START");

#if 0
  // フォントアンロード 不要
  if ( M5.Lcd.fontsLoaded() ) {
    M5.Lcd.unloadFont();
  }
#endif

  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor(0, 0);

  env.setup();

  
  rtc.setAP(ini.wifi_ssid.c_str(), ini.wifi_key.c_str());
  rtc.setNTP();
  rtc.viewTime();

  M5.Lcd.fillScreen(TFT_BLACK);
  //grp.setView( 0,119, 320, 100 );
  //grp.clear();

  // =============== 試しのスプライト
  chart.setColorDepth(8);
  chart.createSprite( LCD_WIDTH, CHART_HEIGHT + 2 );
  chart.fillSprite( TFT_WHITE );
  chart.fillRect( 1, 1, LCD_WIDTH - 2, CHART_HEIGHT, TFT_NAVY );
  chart.setScrollRect( 1, 1, CHART_WIDTH, CHART_HEIGHT, TFT_NAVY );
  chart.setTextColor( TFT_WHITE );
  chart.setCursor( 1, 1 );
  chart.setTextSize( 1 );
  chart.drawFastVLine(CHART_WIDTH + 1, 0, 102, WHITE);
  //chart.drawFastVLine(CHART_WIDTH+2,0,102,WHITE);
}

// スプライト上にドットを描く
int16_t plot2Sprite( uint8_t cnt, float dt, float min, float max, uint32_t color , int16_t oy = -1 ) {
  uint16_t y;
  y = CHART_HEIGHT  -  ( CHART_HEIGHT / ( max - min ) * dt );
  if ( CHART_HEIGHT < dt ) dt = CHART_HEIGHT;
  if ( 0 > dt ) dt = 0;
  Serial.printf("plot  %5.2f [ %5.1f - %5.1f ] ->  %d \r\n", dt, min, max , y );
  if ( oy == -1 ) {
    //chart.drawPixel( CHART_WIDTH-1, y, color );
    chart.drawCircle( CHART_WIDTH - 1, y, 1, color );
  } else {
    chart.drawLine( CHART_WIDTH - 2, oy, CHART_WIDTH - 1, y, color );
  }
  return y;
}


/* ===========================================================================================loop */
void loop() {
  M5.update();
  String buff;

  // スプライトを描画
  // スプライトは書いたままだと表示されなくpushSpriteにて初めて表示
  chart.pushSprite( 0, 120 );

  // シリアル通信を受信したときの処理
  if (Serial2.available()) {
    buff = Serial2.readStringUntil('\n');
    buff.trim();
    if ( buff.length() != 0 ) {
      //Serial.println("Recv Data");
      n_data.decode( buff );
      n_data.view();
      if ( count++ % 60 == 0 ) {
        chart.drawFastVLine(CHART_WIDTH - 1, 0, CHART_HEIGHT, TFT_DARKGREEN);
        chart.drawFastVLine(CHART_WIDTH - 1, 0, 5, TFT_WHITE);
      }
      plot2Sprite(n_data.seq, n_data.templ, 0, 40, TFT_YELLOW);
      plot2Sprite(n_data.seq, n_data.humid, 0, 100, TFT_BLUE);
      plot2Sprite(n_data.seq, n_data.lux, 0, 1400, TFT_GREEN);
      chart.scroll( -2 );
    }
  }

  // ボタンAが押された時の処理
  if (M5.BtnA.wasPressed()) {
    M5.Lcd.setCursor(5, 0);
    M5.Lcd.print("Transmitted : hello");
    // "Hello"をラズパイへ送信する
    //Serial2.write("hello");
    //Serial.println("hello");
  }

  // ボタンBが押された時の処理：Clear
  if (M5.BtnB.wasPressed()) {
    //grp.clear();
    n_data.view();
  }

  // ボタンCが押された時の処理：Clear
  if (M5.BtnC.wasPressed()) {
    M5.Lcd.setTextSize( 2 );
    n_data.dump();
    //grp.clear();
  }

  //本体センサー表示
  env.view();

  // 時刻表示
  rtc.viewTime();
}
