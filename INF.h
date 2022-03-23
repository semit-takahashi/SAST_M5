#ifndef __INF_H__
#define __INF_H__

#include <M5Stack.h>
#include <IniFile.h>
#include "SAST_M5.h"
#include "sensor.h"
#include "display.h"

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
    uint8_t amb_templ;
    uint8_t amb_humid;
    uint8_t amb_avs;
} st_SensINF;

class INF {
    private:
        const char *iniFile = "/setup.ini";

    public:
        // Wifi
        st_wifi wifi[3];

        // SensList
        SensList *slist;

        // Threshold
        SensThresh thresh;

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
        String  QRCode;

        // Threshold TEMP
        float   temp_warn = 0.0;
        float   temp_caut = 0.0;

    public:
        bool load();
        st_wifi getWiFi( uint8_t num );
        void setSensorList( SensList *list );

    private:
        void printErrorMessage(uint8_t e, bool eol = true);
        const char *getValueSTR( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
        bool getValueBOOL( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
        float getValueFLOAT( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
        int16_t getValueINT( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
};


#endif // __INF_H__
