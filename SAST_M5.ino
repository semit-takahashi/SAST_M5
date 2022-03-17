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

// MUTEX（画面描画時利用）
portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;


// センサーデータ
class SensData {
  public:
    float   m_templ = 0.0;
    float   m_humid = 0.0;
    float   m_press = 0.0;
    float   m_als   = 0.0;
    uint8_t m_rssi = 255;
    int8_t  m_batt = -1;   // Batter% (-1 unknown)
    float   m_minute = 0;
    struct tm  m_time;
  private:
    SensData  *node = nullptr;

  public:
    // センサーデータをリンク
    void Append ( SensData *data ) {
      if ( node != nullptr )
        node->Append( data );
      else
        node = data;
      return;
    }

    // 次があれば返す
    SensData *HasNext () {
      if ( node != nullptr ) return node;
      return NULL;
    }

    // 時刻設定
    void setTime( struct tm stime ) {
      m_time = stime;
      m_minute = m_time.tm_hour * 60 + m_time.tm_min + (float)m_time.tm_sec / 60.0;
    }

    // 設定分を返す
    float getMinute() {
      return m_minute;
    }

    // コンストラクタ
    SensData() {
    }

    // デストラクタ
    ~SensData() {
      if ( node != nullptr ) {
        delete node;
        return;
      }
      //Serial.printf("delete %x\r\n",this );
    }

#ifdef TEST
    static void test( netRTC *rtc) {
      uint32_t heap_start = esp_get_free_heap_size();

      Serial.println();
      Serial.printf("SensData TEST - start : %d\r\n", heap_start);

      SensData* ptr = new SensData();
      ptr->setTime( rtc->getTimeStruct() );
      //Serial.printf("d(0000) %x\r\n", ptr);
      for (int i = 1; i < 100; i++ ) {
        SensData *nPtr = new SensData();
        //Serial.printf("d(%04d) %x\r\n",i, nPtr);
        nPtr->setTime( rtc->getTimeStruct() );
        ptr->Append(nPtr);
      }
      //Serial.println("\r\nSensData TEST  - dump ");
      Serial.printf("Free Heap Size = %d\r\n", esp_get_free_heap_size());
      SensData *buff = ptr;
      while ( buff != NULL) {
        //Serial.printf(" buff %x -> time(%f)\r\n", *buff, buff->getMinute() );
        buff = buff->HasNext();
      }
      delete ptr;
      uint32_t heap_done = heap_done = esp_get_free_heap_size();
      Serial.printf("SensData TEST  - done : %d\r\n", heap_done  );
      Serial.printf("Free Heap diff = %d\r\n", heap_start - heap_done);
    }
#endif
};

/*===================================================== Class ENV III SENS ===*/
class envSens {
  private:
    SHT3X   sht30;
    QMP6988 qmp6988;
    bool    use = false;

  public:
    float     templ = 0.0f;
    float     humid = 0.0f;
    uint16_t  pres  = 0;

  public:
    bool setup() {
#if 1
      // ENV Sens III
      if( !qmp6988.init() ) {
        Serial.println("Could not find a valid BQMP6988 sensor, check wiring!");
        use = false;
        return use;
      }
#else
      // ENV Sens II
      if ( !bme.begin(0x76) ) {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        use = false;
        return use;
      }
#endif
      Serial.println("Ready! ENV III Sensor!");
      use = true;
      update();
      return use;
    }

    void update() {
      if ( !use ) return;
      pres = qmp6988.calcPressure() / 100.0;
      if (sht30.get() == 0) {
        templ = sht30.cTemp;
        humid = sht30.humidity;
      }
    }
    bool isExist() {
      return use;
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
    float     als = 0;        // 照度（0〜XXXX lx） ALS：Ambient Light Sensor :
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
      als = buff.toFloat();
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
      sprintf( dat3, "%4.0f Lx", als);
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
      d.printf("als   %6.1f\r\n", als);
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
      sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d.000",
              1900 + atime.tm_year, atime.tm_mon, atime.tm_mday,
              atime.tm_hour, atime.tm_min, atime.tm_sec );
    }
};

/* ============================================================================================================ 描画クラス === */
class drawLCD {
  public:
    // Screen Area DATA
    const uint16_t LCD_WIDTH      = 320;
    const uint16_t LCD_HEIGHT     = 240;
    const uint16_t LCD_TXT_SIZE   = 7;
    const uint16_t CHART_WIDTH    = 320;
    const uint16_t CHART_HEIGHT   = 113;
    const uint8_t  CHART_POS_X    = 0;
    const uint8_t  CHART_POS_Y    = 111;
    const uint16_t PAN_WIDTH      = 106;
    const uint16_t PAN_HEIGHT     = 111;
    const uint8_t  PAN_POS_X0     = 0;
    const uint8_t  PAN_POS_X1     = 107;
    const uint8_t  PAN_POS_X2     = 213;

    //background color
    const uint16_t  BGC_PAN_NOR =  M5.Lcd.color565( 0x00, 0x70, 0x33 );
    const uint16_t  BGC_PAN_LOW =  M5.Lcd.color565( 0x00, 0x14, 0x62 );
    const uint16_t  BGC_PAN_HIG = M5.Lcd.color565( 0xf2, 0x00, 0x51 );
    const uint16_t  BGC_GRAP    = M5.Lcd.color565( 0x00, 0x38, 0x65 );
    const uint16_t  BGC_STAT    = M5.Lcd.color565( 0x00, 0x00, 0x7b );

  private:
    TFT_eSprite chart1      = TFT_eSprite(&M5.Lcd);   // 温度グラフ
    TFT_eSprite panel1      = TFT_eSprite(&M5.Lcd);   // パネル センサー1
    TFT_eSprite panel2      = TFT_eSprite(&M5.Lcd);   // パネル センサー2
    TFT_eSprite panel3      = TFT_eSprite(&M5.Lcd);   // パネル センサー3


    uint16_t    View_Mode     = 0;      // 画面表示モード
    uint16_t    View_Panel    = 0;      // パネル表示ページ
    uint16_t    View_Graph    = 0;      // グラフ表示ページ

    uint16_t x0, y0, x1, y1, wd, ht = 0;
    uint16_t bg_color = TFT_NAVY;       // 背景色
    uint16_t bd_color = TFT_WHITE;    // 境界色

    // must be include <Ticker.h>
    Ticker  drawTick;
    bool    drawAuto = false;

  public:
    void init()
    {
      Serial.println("drawLCD::init()");
      // グラフエリア
      chart1.setColorDepth(8);
      chart1.createSprite( CHART_WIDTH, CHART_HEIGHT );
      chart1.pushImage( 0, 0, CHART_WIDTH, CHART_HEIGHT, bg_graph );
      chart1.setTextColor( TFT_WHITE );
      // パネル1
      panel1.setColorDepth(8);
      panel1.createSprite( PAN_WIDTH, PAN_HEIGHT );
      panel1.pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, bg_pan_nor );
      panel1.setTextColor( TFT_WHITE );
      // パネル2
      panel2.setColorDepth(8);
      panel2.createSprite( PAN_WIDTH, PAN_HEIGHT );
      panel2.pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, bg_pan_nor );
      panel2.setTextColor( TFT_WHITE );
      // パネル3
      panel3.setColorDepth(8);
      panel3.createSprite( PAN_WIDTH, PAN_HEIGHT );
      panel3.pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, bg_pan_nor );
      panel3.setTextColor( TFT_WHITE );

      // 自動更新
      // drawTick.attach( 10 * 1000, autoDraw );

    }


    void draw() {
      //uint32_t  millis_start = millis();
      Serial.println("drawLCD::draw()");

      // スプライトは書いたままだと表示されなくpushSpriteにて初めて表示
      portENTER_CRITICAL(&mutex);       // LCD描画開始

      // Graph View
      chart1.pushSprite( CHART_POS_X, CHART_POS_Y );

      // DataPanel View
      panel1.pushSprite( PAN_POS_X0, 0 );
      panel2.pushSprite( PAN_POS_X1, 0 );
      panel3.pushSprite( PAN_POS_X2, 0 );
      //Serial.println("drawLCD::draw() -- end");

      //Serial.printf("draw() millis:%d\r\n", millis() - millis_start);
      portEXIT_CRITICAL(&mutex);        // LCD描画終了
    }

    void update_test( float templ, float humid, float press, float als , uint16_t minute, SENS_t type) {
      Serial.printf("update_test() t=%f h=%f p=%f a=%f", templ, humid, press, als );
      drawPanel( &panel1, templ, humid, press, als , PANEL_t::NORM , SENS_t::TH1);
      drawGraph( &chart1, templ, TFT_YELLOW, minute, GRAPH_t::TEMPL );
    }

    void update_test2( float templ, float humid, float press, float als , uint16_t minute, SENS_t type) {
      Serial.printf("update_test2() t=%f h=%f p=%f a=%f", templ, humid, press, als );
      drawPanel( &panel2, templ, humid, press, als , PANEL_t::NORM, SENS_t::Lazurite );
      drawGraph( &chart1, templ, TFT_YELLOW, minute, GRAPH_t::TEMPL );
    }

    void drawPanel( TFT_eSprite *pan, float templ, float humid, float press, float als , PANEL_t mode, SENS_t type)
    {
      char    str_templ[5];
      char    str_humid[5];
      char    str_templ_l[2];
      char    str_humid_l[2];
      char    str_press[5];
      char    str_als[5];
      sprintf( str_templ,   "%3d", (int)templ );
      sprintf( str_humid,   "%3d", (int)humid );
      sprintf( str_templ_l, "%1d", ftoa1(templ) );
      sprintf( str_humid_l, "%1d", ftoa1(humid) );
      sprintf( str_press,   "%4d", (int)press );
      sprintf( str_als,     "%4d", (int)als );
      uint16_t fC = TFT_WHITE;
      uint16_t bC = BGC_PAN_NOR;

      Serial.printf("mode = %d PAN=%x \r\n", (int)mode, PANEL_img[(int)mode]);
      pan->pushImage( 0, 0, PAN_WIDTH, PAN_HEIGHT, PANEL_img[(int)mode] );

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

      if ( type == SENS_t::TH1 ) {
        // draw Pressure
        pan->drawChar( 40, 96 , str_press[0], fC, bC, 2 );
        pan->drawChar( 52, 96 , str_press[1], fC, bC, 2 );
        pan->drawChar( 64, 96 , str_press[2], fC, bC, 2 );
        pan->drawChar( 76, 96 , str_press[3], fC, bC, 2 );
        pan->drawChar( 87, 101 , 'h', fC, bC, 1 );
        pan->drawChar( 94, 101 , 'P', fC, bC, 1 );
        pan->drawChar(100, 101 , 'a', fC, bC, 1 );
      } else {
        pan->drawChar( 40, 96 , str_als[0], fC, bC, 2 );
        pan->drawChar( 52, 96 , str_als[1], fC, bC, 2 );
        pan->drawChar( 64, 96 , str_als[2], fC, bC, 2 );
        pan->drawChar( 76, 96 , str_als[3], fC, bC, 2 );
        pan->drawChar( 94, 101 , 'L', fC, bC, 1 );
        pan->drawChar(100, 101 , 'X', fC, bC, 1 );
      }

      //BATT
      TFT_eSprite batt = TFT_eSprite(pan);
      batt.createSprite( BATT_Width, BATT_Height );
      batt.pushImage( 0, 0, BATT_Width, BATT_Height, BATT_img[0] );
      batt.pushSprite( 86, 3, TFT_GREEN );
      //batt.deleteSprite();

      //ANT
      TFT_eSprite antn = TFT_eSprite(pan);
      antn.createSprite( ANT_Width, ANT_Height );
      antn.pushImage( 0, 0, ANT_Width, ANT_Height, ANT_img[0] );
      antn.pushSprite( 64, 3, TFT_GREEN );
      //antn.deleteSprite();

    }

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




    void setAutoDraw( bool flag ) {
      drawAuto = flag;
    }

    void change_Panel() {
      View_Panel = View_Panel % 2;
    }

    void change_Mode() {
      View_Mode = View_Mode % 3;
    }

    void change_Graph() {
      View_Graph = View_Graph % 3;
    }

    void Update()
    {
      // センサーデータのパネルとグラフ書き込み
      // 表示順 設定
      // パネルデータ描画（3パネル）
      //    閾値確認して、必要なパネルに切り替える
      //    データのdraw

      // グラフ種別確認
      // 日付が変わったかを確認
      // グラフデータ描画
    }

    void reDraw()
    {
      draw();
    }

  private:
    void autoDraw() {
      if ( !drawAuto ) return;
      Update();
      draw();
    }

    uint8_t ftoa1( float val )
    {
      return (uint8_t)( (val - (int)val ) * 10 );
    }
};

/* ======================================================================================= Globals */
// 受信データ最新
L902J n_data;

//環境センサ
envSens env;

// 時計
netRTC rtc;

// 画面クラス
drawLCD LCD;

// INI呼び出し
INF ini;


// 描画回数
uint32_t count = 0;

// 画面輝度
const uint8_t brightness[] = { 0, 5, 20 ,80 ,200 };
uint8_t light = 4;

// タイマー割込
Ticker  int_SBar;
bool sClock = false;    // 秒点滅用フラグ
Ticker  int_EnvSens;

/* ==================================================================================== ENV Sensor BAR */
void updateEnvSensor() {
  if ( env.isExist() ) {
    env.update();
    LCD.update_test( env.templ , env.humid, env.pres, 0.0, rtc.getMinute(), SENS_t::TH1 );
    LCD.draw();
  }
}

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


/* === 画面輝度設定 4~0 === */

void setBrightness( int brt = -1 )
{
  if( brt == -1 ) {
    if ( light-- == 0 ) light = 4;
  } else {
    light = brt;
  }
  Serial.printf("Brightness %d\r\n",light);
  M5.Lcd.setBrightness(brightness[light]);
}

/* =========================================================================================setup */
void setup() {
  M5.begin();
  M5.Power.begin();

  // シリアル通信機能の設定
  //Serial.begin(115200);

  // INI呼び出し
  ini.load();

  // シリアル通信機能2の設定
  // Serial2.begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert)
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  // 初期画面設定
  setBrightness( 2 );
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextSize(1);
  M5.Lcd.fillScreen(TFT_NAVY);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_NAVY );
  M5.Lcd.setCursor(2, 2);
  Serial.printf("SAST M5 Ver.%s\r\n", _VERSION_ );
  M5.Lcd.printf("SAST M5 Ver.%s\r\n", _VERSION_ );
  if (!M5.Power.canControl()) M5.Lcd.println(" ~~ Can't Power Control ~~");
  delay(500);

  // 時刻設定(Wi-Fi接続)
  for( int i=0; i < 3; i++ ) {
    st_wifi ap;
    ap = ini.getWiFi( i );
    rtc.setAP( ap.ssid.c_str(), ap.key.c_str() );
    if( rtc.setNTP() ) {
      continue;
    }
  }

  // 本体付属センサー初期化（ENV II）
  env.setup();

  // 初期画面設定
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.fillScreen(BGC_STAT);
  //M5.Lcd.drawBmpFile( SD, "/SAST_BACK.bmp", 0,0 );

  // LCD初期化
  LCD.init();
  LCD.draw();

  // タイマー割込設定　ステータスバー更新
  int_SBar.attach(1, drawStatusBar);

  // タイマー割込設定　ENVIIセンサー（割込負荷分散）
  if ( env.isExist() ) {
    int_EnvSens.attach( 60, updateEnvSensor );
    // 初期更新
    updateEnvSensor();
  }
}

/*
  #if 0
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
  #endif
*/
/* ===========================================================================================loop */
void loop() {
  M5.update();

#ifdef TEST
  SensData::test( &rtc );
#endif

  // シリアル通信を受信したときの処理
  String buff;
  if (Serial2.available()) {
    buff = Serial2.readStringUntil('\n');
    buff.trim();
    if ( buff.length() != 0 ) {
      //Serial.println("Recv Data");

      n_data.decode( buff );
      LCD.update_test2( n_data.templ, n_data.humid, 0.0, n_data.als, rtc.getMinute(), SENS_t::Lazurite );
      LCD.draw();


      // アドレス毎にデータ送信
    }

  }

  // IBS-TH!のデータ受信
  // スキャンしてデータ取得
  // MAC毎にデコード呼び出し



  // ボタンAが押された時の処理
  if (M5.BtnA.wasPressed()) {
    setBrightness();
  }

  // ボタンBが押された時の処理：Clear
  if (M5.BtnB.wasPressed()) {
    long templ = random( 0 , 500 );
    long humid = random( 0, 100 );
    long press = random( 980, 1200 );
    long avs   = random( 0, 10000 );
    LCD.update_test( (float)templ / 10.0, humid, press, avs , rtc.getMinute() , SENS_t::TH1 );
    LCD.draw();
  }

  // ボタンCが押された時の処理：Clear
  if (M5.BtnC.wasPressed()) {
    M5.Lcd.setTextSize( 2 );
    n_data.dump();
    LCD.draw();
  }
}
