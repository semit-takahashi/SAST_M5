/**
 * @file INF.cpp
 * @author F.Takahashi (fumihito.takahashi@sem-it.com)
 * @brief SAST M5用 INIファイル読み取りクラス
 * @version 2.1
 * @date 2021-08-20
 * 
 * @copyright Copyright (c) 2021 SEM-IT
 */
#include "INF.h"

/**
 * @brief 初期値取得
 */
bool INF::load(){
    const size_t  buff_len = 310;
    char          buff[buff_len];
    String        sect;

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
      M5.Lcd.printf("ERROR Ini file [%s] does not exist. \r\n STOP!", iniFile);
      while (1);
    }
    Serial.printf("INI file [ %s ] exists\r\n", iniFile);

    // Check File is Valid.
    if (!in.validate(buff, buff_len)) {
      Serial.printf("ini file %s not valid\r\n", in.getFilename());
      M5.Lcd.printf("ini file %s not valid\r\n", in.getFilename());
      printErrorMessage(in.getError());
      // Cannot do anything else
      while (1);
    }

    // ======== Wi-Fi
    wifi[0].ssid = getValueSTR( in, "Wi-Fi", "SSID1", buff, buff_len );
    wifi[0].key  = getValueSTR( in, "Wi-Fi", "KEY1", buff, buff_len );
    wifi[1].ssid = getValueSTR( in, "Wi-Fi", "SSID2", buff, buff_len );
    wifi[1].key  = getValueSTR( in, "Wi-Fi", "KEY2", buff, buff_len );
    wifi[2].ssid = getValueSTR( in, "Wi-Fi", "SSID3", buff, buff_len );
    wifi[2].key  = getValueSTR( in, "Wi-Fi", "KEY3", buff, buff_len );

    // ======== LINE
    sect = "LINE";
    LINE_token = getValueSTR( in, sect.c_str(), "URL", buff, buff_len );
    LINE_URL = getValueSTR( in, sect.c_str(), "token", buff, buff_len );

    // ======== Google Sopreadsheet
    sect = "Google";
    GS_token = getValueSTR( in, sect.c_str(), "token", buff, buff_len );

    // ======== QR Code URL
    sect = "QRCode";
    QRCode = getValueSTR( in, sect.c_str(), "URL", buff, buff_len );

    // ======== Threshold temp
    sect = "THRESH_TEMP";
    temp_warn = getValueFLOAT( in, sect.c_str(), "WARN", buff, buff_len );
    temp_caut = getValueFLOAT( in, sect.c_str(), "CAUTION", buff, buff_len );

    // ======== Sensors 
    uint8_t max_sens = 6;
    const char* sens_name[] = {"SENSORS_1","SENSORS_2","SENSORS_3","SENSORS_4","SENSORS_5","SENSORS_6"};
    for( int i=0; i < max_sens; i++ ) {
        String type =  getValueSTR( in, sens_name[i], "MAC", buff, buff_len );
        if( type == "LAZURITE" ) {
          sens[i].stype = SENS_t::Lazurite;
        }else if( type == "TH1" ) {
          sens[i].stype = SENS_t::TH1;
        }else {
          // それ以外の場合はセンサーが無いとする
          sens[i].stype = SENS_t::None;
          continue;

        }
        sens[i].id = getValueSTR( in, sens_name[i], "ADDR", buff, buff_len );
        sens[i].name = getValueSTR( in, sens_name[i], "name", buff, buff_len );
        sens[i].amb_templ = getValueSTR( in, sens_name[i], "templ", buff, buff_len );
        sens[i].amb_humid = getValueSTR( in, sens_name[i], "humid", buff, buff_len );
        sens[i].amb_avs = getValueSTR( in, sens_name[i], "als", buff, buff_len );
    }

}

/**
 * @brief WiFi設定情報取得
 * @param num 0〜3（設定情報）
 * @return st_wifi 
 */
st_wifi INF::getWiFi( uint8_t num ) 
{
    return wifi[num];
}


// INI Value String 取得
const char *INF::getValueSTR( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len ) {
    if ( in.getValue( sect, name, buff, buff_len ) ) {
      DMSGf("[%s] %s: %s\r\n", sect, name, buff);
      return buff;
    } else {
      DMSGf("Not found [%s] [%s] : ", sect, name );
      printErrorMessage(in.getError());
      return NULL;
    }
}

// INI Bool 取得
bool INF::getValueBOOL( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len ) {
    bool val = false;
    if ( in.getValue( sect, name, buff, buff_len , val ) ) {
      DMSGf("[%s] [%s]: %s\r\n", sect, name, val ? "TRUE":"FALSE");
      return val;
    } else {
      DMSGf("Not found [%s] [%s] : ", sect, name );
      printErrorMessage(in.getError());
      return false;
    }
}

// INI float 取得
float INF::getValueFLOAT( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len ) {
    float val = 0.0f;
    if ( in.getValue( sect, name, buff, buff_len , val) ) {
      DMSGf("[%s] [%s]: %s\r\n", sect, name, buff);
      return val;
    } else {
      DMSGf("Not found [%s] [%s] : ", sect, name );
      printErrorMessage(in.getError());
      return 0.0f;
    }
}

// INI float 取得
int16_t INF::getValueINT( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len ) {
    int val = 0;;
    if ( in.getValue( sect, name, buff, buff_len , val) ) {
      DMSGf("[%s] [%s]: %s\r\n", sect, name, buff);
      return val;
    } else {
      DMSGf("Not found [%s] [%s] : ", sect, name );
      printErrorMessage(in.getError());
      return 0;
    }
}

// エラーメッセージ変換
void INF::printErrorMessage(uint8_t e, bool eol ) {
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
