/**
 * @file netRTC.h
 * @author F.Takahashi (fumihito.takahashi@sem-it.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __NETRTC_H__
#define __NETRTC_H__

#include "SAST_M5.h"
#include <M5Stack.h>
#include <WiFi.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include "Ambient.h"

/**
 * @brief NTP
 * 
 */
class netRTC {
  private:
// Ambient
Ambient AMB;

//WiFi Client
WiFiClient Client;


    // UpdateTiming
    const unsigned long rst_hour = 600 * 1000;    // 10minute
    //const unsigned long rst_hour = 2.16e7;    // 6hour

    bool          IsWiFi_Set = false;
    String        ssid;
    String        key;
    const char*   ntp1 = "ntp.jst.mfeed.ad.jp";
    const char*   ntp2 = "ntp.nict.jp";
    const char*   ntp3 = "time.cloudflare.com";
    const long    gmtOffset_sec = 3600 * 9;   // JST +9:00
    uint16_t      daylightOffset_sec = 0;
    struct tm     tf;                 // 現在のtm構造体
    time_t        tm;                 // 現在のtime_t
    char          str_stime[16];      // 短い時間 MM/DD hh:mm
    char          str_ltime[24];      // 長い時間 YYYY-MM-DD hh:mm:ss.ttt
    uint16_t      minute;           
    uint16_t      rst_count = 0;      // reset counter
    bool          amb_use = false;    // use Ambient
    const char*   LINE_Notify = "notify-api.line.me"; 
    String        LINE_token;
    bool          LINE_use = false;   // use LINE
    String        GAS_URL;            //
    bool          GAS_use = false;    // use Google Spreadsheet

  public:
    uint8_t       _cnt_error = 0;           // 接続エラー回数


  public:
    void        setAP( const char *wifi_ssid, const char *wifi_key );
    bool        setNTP();
    void        calc();
    const char  *getTimeSTR();
    void        reflesh();
    struct tm   getTimeStruct();
    uint16_t    getMinute();
    void        beep();
    time_t      getTimeRAW();
    bool        isSet();
    bool        isConnect();
    double      getTimeDiffer( const time_t srcTime );
    static bool makeTimeString( const time_t srcTime , char *timeSTR );
    bool        isElapsed( const time_t srcTime, const int min = 5 );

    bool        connect();
    bool        disconnect();

    void        setAmbient( Ambient *amb );
    void        setupAmbient( const int channel, const char* write );
    void        setupNotify( const char* token );
    void        setupGAS( const char* URL );
    bool        sendAmbient( st_AMB dt[] );
    bool        sendGAS( String data );
    bool        sendNotify( String mess );

};

#endif //__NETRTC_H_
