#ifndef __INF_H__ 
#define __INF_H__

#include <M5Stack.h>
#include <IniFile.h>

struct thrd {
  bool use = false;
  float min;
  float max;
};

struct SENS_TH1 {
  String  MAC;
  String  name;
  String  amb_templ;
  String  amb_humid;
  struct thrd th_templ;
  struct thrd th_humid;
};

struct SENS_920J {
  uint16_t  ADDR;
  String  name[60];
  String  amb_templ[3];
  String  amb_humid[3];
  String  amb_als[3];
  String  amb_press[3];
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

public:
    bool load();
    const char *getValueSTR( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
    bool getValueBOOL( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
    float getValueFLOAT( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len );
    void printErrorMessage(uint8_t e, bool eol = true);
};


#endif // __INF_H__
