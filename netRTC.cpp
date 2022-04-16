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
  IsWiFi_Set = true;
  Serial.printf("setAP() %s / %s\n",wifi_ssid, wifi_key );
}

/**
 * @brief NTPを設定して時刻を取得
 * 
 * @return true 
 * @return false 
 */
bool netRTC::setNTP()
{
  if ( !IsWiFi_Set ) return false;
  uint8_t cnt = 20;
  IPAddress ip;

  // connect to WiFi
  M5.Lcd.printf("connecting to\r\n %s ", ssid.c_str());
  Serial.printf("Connecting to %s ", ssid.c_str());
  WiFi.begin(ssid.c_str(), key.c_str());
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
  M5.Lcd.print(" CONNECTED\n\n ["); M5.Lcd.print(ip);M5.Lcd.println("]\n");
  Serial.print(" CONNECTED ["); Serial.print(ip);Serial.println("]");
  
  // Set netRTC time to local
  IsWiFi_Set = true;
  configTime(gmtOffset_sec, daylightOffset_sec, ntp1, ntp2, ntp3 );
  calc();
  M5.Lcd.println( str_stime );
  Serial.printf("%s ( %d ) \n",str_stime, tm );

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
  _minute = tf.tm_hour * 60 + tf.tm_min;

  tm = mktime(&tf);
}

/**
 * @brief calc()時点ののショート時間（M/D HH:MM)文字列を返す
 * @return const char* 
 */
const char* netRTC::getTimeSTR(){
  return str_stime;
}

/**
 * @brief calc()時点の1日の経過分を返す
 * @return uint16_t 
 */
uint16_t netRTC::getMinute() {
  return _minute;
}

/**
 * @brief calc()時点のUNIX Timeを取得 LONG型 
 * @return time_t 
 */
time_t netRTC::getTimeRAW() {
  return tm;
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
    WiFi.begin(ssid.c_str(), key.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      if ( cnt-- == 0 ) {
        Serial.println("\r\nERROR Not Connect skip!!\r\n");
        Serial.println(WiFi.status());
        return;
      }
      delay(500);
    }
    configTime(gmtOffset_sec, daylightOffset_sec, ntp1, ntp2, ntp3 );
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println(" .. done.");
  }
}


/**
 * @brief エラーBEEPを発生
 */
void netRTC::beep(){
  M5.Speaker.begin();       // 呼ぶとノイズ(ポップ音)が出る 
  M5.Speaker.setVolume(1);  // 0は無音、1が最小、8が初期値(結構大きい)
  M5.Speaker.beep();        // ビープ開始
  delay(80);               // 100ms鳴らす(beep()のデフォルト)
  M5.Speaker.mute();        //　ビープ停止
}

/**
 * @brief NTPが設定済み換えを返す
 * @return true 
 * @return false 
 */
bool netRTC::isSet(){
  return IsWiFi_Set;
}

bool netRTC::isConnect() {
  return WiFi.status() == WL_CONNECTED ? true : false;
}

/**
 * @brief 指定時刻と現在時刻の経過秒数を返す
 * @param srcTime  time_t型の指定時刻
 * @return double 経過秒数
 */
double netRTC::getTimeDiffer( const time_t srcTime ) {
    struct tm lTime;
    getLocalTime(&lTime);
    time_t lt = mktime(&lTime);
    return difftime( lt, srcTime );
}

/**
 * @brief 指定した分が経過したか？
 * @param srcTime  time_t型の指定時刻
 * @param min 比較する分（省略時5分）
 * @return true 経過済
 * @return fasle 未経過
 */
bool netRTC::isElapsed( const time_t srcTime, const int min ) {
    if( srcTime == 0 ) return true;    // srcTimeが未設定の場合は経過済みと見なす
    
    double sec = getTimeDiffer( srcTime );
    if( sec /60 > min ) return true;
    return false;
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

/**
 * @brief 無線LANに接続
 * 
 * @return true 
 * @return false 
 */
bool netRTC::connect() {
  if ( !IsWiFi_Set ) return false;
  if ( WiFi.status() == WL_CONNECTED ) return true;
  uint8_t cnt = 20;
  IPAddress ip;

  Serial.printf("Connecting to %s ", ssid.c_str());
  WiFi.begin(ssid.c_str(), key.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    if ( cnt-- == 0 ) {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      Serial.printf(" Connect ERROR! (%d)\r\n", WiFi.status());
      beep();
      return false;
    }
    delay(500);
    Serial.print(".");
  }
  ip = WiFi.localIP();
  Serial.print(" CONNECTED\n\n ["); Serial.print(ip);Serial.println("]\n");

  return true;
}

/**
 * @brief 無線LANを切断
 * 
 * @return true  成功
 * @return false 無線LAN接続してない
 */
bool netRTC::disconnect() {
  if ( !IsWiFi_Set ) return false;
  if ( WiFi.status() != WL_CONNECTED ) return true;

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi disconnect.");
  return true;
}

/**
 * @brief Ambientへのグラフ作成処理初期化
 * 
 * @param channel Ambientチャンネル
 * @param write ライトキー
 */
void netRTC::setupAmbient( const int channel, const char* write) {
  connect();
  WiFi.setAutoConnect( true ); //TODO 自動接続？
  AMB.begin( channel, write, &Client );
  amb_use = true;

}
/*
void netRTC::setAmbient( Ambient *amb ) {
  _AMB = amb;
}
*/
/**
 * @brief Ambientにデータ送信
 * @param dt データ構造対
 * @return true 成功
 * @return false 失敗
 */
bool netRTC::sendAmbient( st_AMB dt[]) {
  Serial.print("netRTC::sendAmbient()");
  for( int i=0; i < MAX_AMB; i++) {
    if( dt[i].use ) {
      //Serial.printf(" %d = %s\n", i+1, String( dt[i].dt, 1).c_str() );
      AMB.set( i+1, String( dt[i].dt, 1).c_str());
    }
  }
  bool res = AMB.send();
  if( res == false ) {
    Serial.printf("-> ERROR(%d) ",_cnt_error );
    if( ++_cnt_error == 10 ) {
      //10回連続で通信不可の場合は強制リセット
      Serial.println("Reset M5Stack!");
      beep();
      beep();
      beep();
      M5.Power.reset();
    }
  }else{
    _cnt_error = 0;      // 正常ならエラーカウントを0に
  }
  Serial.printf("-> %s (%d)\n",res ? "OK" : "NG", AMB.status);
  return res;
}


/**
 * @brief GASの初期設定
 * @param URL 
 */
void netRTC::setupGAS( const char* URL ) {
  GAS_URL = URL;
  GAS_use = true;
}

bool netRTC::sendGAS( String data ){

}


/**
 * @brief LINE通知の情報を設定する
 * @param URL 通知接続先URL
 * @param token トークン（）
 */
void netRTC::setupNotify( const char* token ) {
  LINE_token = token;
  LINE_use = true;
}

/**
 * @brief LINE Notifyの通知（使う前にsetupNotifyを実行する）
 * @param mess 
 * @return true 
 * @return false 
 */
bool netRTC::sendNotify( String mess ){
  Serial.println("sendNotify()");
  if ( !LINE_use ) return false;

  // 22時〜5時の間は通知を停止
  calc();
  uint16_t hour = _minute / 60 ;
  Serial.printf("Notify %d:00 o'clock(%d minute)\n",hour, _minute );
  if( hour <= 5  || hour >= 22 ) {
    Serial.println("Notify - Night mode..zzz..");
    return false;
  }

  // 接続を再確認して未接続の場合は再接続処理
  if( WiFi.status() != WL_CONNECTED ) {
    Serial.println("Retry Wi-Fi connect...");
    disconnect();
    delay(500);
    connect();
  }

  WiFiClientSecure client;

  if (!client.connect(LINE_Notify, 443)) {
    Serial.printf("sendNotify:connection Error!! %s\n",LINE_Notify);
    delay(2000);
    return false;
  }
  //Serial.println("connect..");

  String query = String("message=") + mess;
  String request = String("") +
               "POST /api/notify HTTP/1.1\r\n" +
               "Host: " + LINE_Notify + "\r\n" +
               "Authorization: Bearer " + LINE_token + "\r\n" +
               "Content-Length: " + String(query.length()) +  "\r\n" + 
               "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
                query + "\r\n";
  //Serial.println(request);

  client.print(request);
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Response : headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  Serial.println(line);
  return true;
}
