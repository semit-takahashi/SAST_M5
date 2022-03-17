/**
 * @file netRTC.cpp
 * @author F.Takahashi (fumihito.takahashi@sem-it.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "netRTC.h"

/**
 * @brief WiFiアクセスポイント情報を設定
 * @param wifi_ssid 
 * @param wifi_key 
 */
void netRTC::setAP( const char *wifi_ssid, const char *wifi_key )
{
  ssid = wifi_ssid;
  key = wifi_key;
  IsSet = true;
  Serial.println( ssid );
  Serial.println( key );
}

/**
 * @brief NTPを設定して時刻を取得
 * 
 * @return true 
 * @return false 
 */
bool netRTC::setNTP()
{
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
      beep();
      return false;
    }
    delay(500);
    M5.Lcd.printf(".");
    Serial.print(".");
  }
  ip = WiFi.localIP();
  M5.Lcd.print(" CONNECTED\r\n -> "); M5.Lcd.println(ip);
  Serial.print(" CONNECTED\r\n -> "); Serial.println(ip);
  
  // Set netRTC time to local
  IsSet = true;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  calc();
  M5.Lcd.println( str_stime );
  Serial.println( str_stime );
  //delay(1000);

  //disconnect WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

/**
 * @brief 現在時刻を計算して、文字列を生成
 * 
 */
void netRTC::calc() {
  if (!getLocalTime(&tf)) {
    M5.Lcd.println("Failed to obtain time");
    Serial.println("ERROR: Faild to obtain time");
    return;
  }
  sprintf(str_stime, "%2d/%2d %2d:%02d", tf.tm_mon+1, tf.tm_mday, tf.tm_hour, tf.tm_min );
  sprintf(str_ltime, "%04d-%02d-%02d %02d:%02d:%02d.000", 1900 + tf.tm_year, tf.tm_mon+1, tf.tm_mday, tf.tm_hour, tf.tm_min, tf.tm_sec );
  minute = tf.tm_hour * 60 + tf.tm_min;
}

const char* netRTC::getTimeSTR(){
  return str_stime;
}

uint16_t netRTC::getMinute() {
  return minute;
}


/**
 * @brief UNIX Timeを取得 LONG型 
 * @return time_t 
 */
time_t netRTC::getTimeRAW()
{
  //setTime();
  time_t t;
  time(&t);
  return t;
}

// netRTC時間の再設定を実施
void netRTC::reflesh() {
  // 更新時間を経過した？
  unsigned int hour = millis() / rst_hour;
  Serial.printf("ref: %d(%fd)\r\n", hour, rst_count );
  // タイマーが戻った？（50日経過した？）
  if ( hour == 0 && rst_count != 0 ) rst_count = 0;
  if ( hour > rst_count ) {
    uint8_t cnt = 20;
    Serial.print("Reflesh TIME: set netRTC ");
    rst_count++;
    WiFi.begin(ssid, key);
    while (WiFi.status() != WL_CONNECTED) {
      if ( cnt-- == 0 ) {
        Serial.println("\r\nERROR Not Connect skip!!\r\n");
        Serial.println(WiFi.status());
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

void netRTC::beep()
{
  M5.Speaker.begin();       // 呼ぶとノイズ(ポップ音)が出る 
  M5.Speaker.setVolume(1);  // 0は無音、1が最小、8が初期値(結構大きい)
  M5.Speaker.beep();        // ビープ開始
  delay(100);               // 100ms鳴らす(beep()のデフォルト)
  M5.Speaker.mute();        //　ビープ停止
}

/*
  void netRTC::viewTime()
  {
    calc();
    uint8_t txtSize = 2;
    M5.Lcd.setTextColor( TFT_DARKGREY, TFT_BLACK);
    M5.Lcd.setTextSize( txtSize );
    uint16_t x = LCD_WIDTH - M5.Lcd.textWidth(str_stime);
    uint16_t y = LCD_HEIGHT - txtSize * LCD_TXT_SIZE;
    M5.Lcd.drawString( str_stime, x, y );
  }
*/
