#ifndef __INF_H__
#define __INF_H__

#include <M5Stack.h>
#include <IniFile.h>
#include "SAST_M5.h"

#define __DEBUG
#ifdef __DEBUG
#define DMSGf(...) Serial.printf(__VA_ARGS__)
#define DMSG(X) Serial.print(X)
#else
#define DMSGf(...) 
#define DMSG(X) 
#endif

typedef struct{
  String ssid;
  String key;
}st_wifi;

typedef struct{
  bool use = false;
  float warn;
  float caut;
}st_thrsh;

typedef struct{
  SENS_t stype;
  String name;
  String id;
  String amb_templ;
  String amb_humid;
  String amb_avs;
} st_SensINF;

class INF {
  private:
    const char *iniFile = "/setup.ini";

  public:
    // Wifi
    st_wifi wifi[3];

    //Sensors
    st_SensINF sens[6];

    // ambient
    String  amb_chID;
    String  amb_wKey;
    String  amb_rKey;

    // LINE
    String  LINE_token;
    String  LINE_URL;

    // Google Spredseet
    String  GS_token;

    // QRコードURL
    String   QRCode;

    // Threshold TEMP
    float   temp_warn = 0.0;
    float   temp_caut = 0.0;

  public:
    bool load();
    const char *getValueSTR( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
    bool getValueBOOL( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
    float getValueFLOAT( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
    int16_t getValueINT( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
    st_wifi getWiFi( uint8_t num );

  private:
    void printErrorMessage(uint8_t e, bool eol = true);
};


#endif // __INF_H__
