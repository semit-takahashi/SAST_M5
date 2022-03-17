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

#include <M5Stack.h>
#include <WiFi.h>
#include <time.h>

/**
 * @brief NTP
 * 
 */
class netRTC {
  private:

    // UpdateTiming
    const unsigned long rst_hour = 600 * 1000;    // 10minute
    //const unsigned long rst_hour = 2.16e7;    // 6hour

    bool          IsSet = false;
    const char*   ssid;
    const char*   key;
    const char*   ntpServer =  "ntp.jst.mfeed.ad.jp";
    const long    gmtOffset_sec = 3600 * 9;   // JST +9:00
    uint16_t      daylightOffset_sec = 0;
    struct tm     tf;                 // 現在のtm構造体
    char          str_stime[16];      // 短い時間 MM/DD hh:mm
    char          str_ltime[24];      // 長い時間 YYYY-MM-DD hh:mm:ss.ttt
    uint16_t      minute;           
    uint16_t      rst_count = 0;      // reset counter

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

};

#endif //__NETRTC_H_
